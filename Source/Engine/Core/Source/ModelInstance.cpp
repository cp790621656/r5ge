#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves the world transformation matrix
//============================================================================================================

const Matrix43& ModelInstance::GetMatrix() const
{
	if (mRecalculate)
	{
		mRecalculate = false;
		mMatrix.SetToTransform( mAbsolutePos, mAbsoluteRot, mAbsoluteScale );
	}
	return mMatrix;
}

//============================================================================================================
// Updates the pointer to the instanced model, keeping track of the number of instances
//============================================================================================================

void ModelInstance::SetModel (Model* model, bool runOnSerialize, bool threadSafe)
{
	if (mModel != model)
	{
		mIsDirty = true;
		if (mModel != 0) mModel->_Decrement();
		mModel = model;
		
		if (mModel != 0)
		{
			mModel->_Increment();

			if (runOnSerialize)
			{
				TreeNode& onSerialize = mModel->GetOnSerialize();

				if (onSerialize.HasChildren())
				{
					SerializeFrom(onSerialize, false, threadSafe);
				}
			}
		}
	}
}

//============================================================================================================
// Updates the transformation matrix
//============================================================================================================

void ModelInstance::OnUpdate()
{
	if (mModel != 0)
	{
		if (mModel->IsDirty()) mIsDirty = true;
		mRelativeBounds = mModel->GetBounds();
	}
	if (mIsDirty) mRecalculate = true;
}

//============================================================================================================
// Fills the render queues
//============================================================================================================

bool ModelInstance::OnFill (FillParams& params)
{
	if (mModel != 0)
	{
		float dist = (mAbsolutePos - params.mCamPos).Dot();
		ModelTemplate::Limbs& limbs = mModel->GetAllLimbs();

		// If we have more than one limb, we should group by material (example: tree is made up of
		// trunk and canopy limbs). Doing this saves texture, matrix, and shader switches.
		if (limbs.GetSize() > 1)
		{
			mModel->Lock();
			{
				for (uint i = 0, imax = limbs.GetSize(); i < imax; ++i)
				{
					Limb* limb = limbs[i];

					if (limb->IsValid())
					{
						bool isVisible (true);

						// Models with 3 or more limbs should cull their limbs individually,
						// but only if the model is referenced just once. This is a quick-and-dirty
						// optimization for complex scenes created as one large model.

						if ((limbs.GetSize() > 2) && (mModel->GetNumberOfReferences() == 1))
						{
							Bounds bounds (limb->GetMesh()->GetBounds());
							bounds.Transform(mAbsolutePos, mAbsoluteRot, mAbsoluteScale);
							isVisible = params.mFrustum.IsVisible(bounds);
						}

						// Update the limb's visibility flag
						limb->SetVisible(isVisible);

						// If the limb is visible, add this model to the draw queue
						if (isVisible)
						{
							IMaterial* mat = limb->GetMaterial();
							params.mDrawQueue.Add(mLayer, this, mat->GetTechniqueMask(), mat->GetUID(), dist);	
						}
					}
				}
			}
			mModel->Unlock();
		}
		else if (limbs.IsValid())
		{
			// If we only have 1 limb, it makes sense to group by model instead
			params.mDrawQueue.Add(mLayer, this, mModel->GetMask(), mModel->GetUID(), dist);
			limbs[0]->SetVisible(true);
		}
	}
	return true;
}

//============================================================================================================
// Draw the object using the specified technique
//============================================================================================================

uint ModelInstance::OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, bool insideOut)
{
	IGraphics* graphics = mCore->GetGraphics();

	// Automatically normalize normals if the scale is not 1.0
	graphics->SetNormalize( Float::IsNotEqual(mAbsoluteScale, 1.0f) );

	// Set the model's world matrix so the rendered objects show up in their proper place
	graphics->SetModelMatrix( GetMatrix() );

	// Draw the model
	return mModel->_Draw(group, graphics, tech);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void ModelInstance::OnSerializeTo (TreeNode& node) const
{
	if (mModel != 0)
	{
		node.AddChild( mModel->GetClassID(), mModel->GetName() );
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool ModelInstance::OnSerializeFrom (const TreeNode& node)
{
	const String&	tag   = node.mTag;
	const Variable&	value = node.mValue;

	if ( tag == Model::ClassID() )
	{
		String name (value.IsString() ? value.AsString() : value.GetString());
		Model* model = mCore->GetModel(name, true);
		ModelTemplate* temp = model->GetSource();

		if (temp == 0)
		{
			temp = mCore->GetModelTemplate(name, true);
			model->SetSource(temp);
			model->SetSerializable(false);
		}

		SetModel(model, true, false);
		return true;
	}
	return false;
}