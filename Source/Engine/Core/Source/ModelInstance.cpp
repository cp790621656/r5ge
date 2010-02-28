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

void ModelInstance::SetModel (Model* model)
{
	if (mModel != model)
	{
		mIsDirty = true;
		if (mModel != 0) mModel->_Decrement();
		mModel = model;
		if (mModel != 0) mModel->_Increment();
	}
}

//============================================================================================================
// Updates the transformation matrix
//============================================================================================================

void ModelInstance::OnUpdate()
{
	if (mIsDirty)
	{
		mRecalculate = true;
		mRelativeBounds = mCullBounds;

		if (mModel != 0)
		{
			mRelativeBounds.Include( mModel->GetBounds() );
		}
	}
}

//============================================================================================================
// Fills the render queues
//============================================================================================================

bool ModelInstance::OnFill (FillParams& params)
{
	if (mModel != 0)
	{
		float dist = (mAbsolutePos - params.mCamPos).Dot();
		params.mDrawQueue.Add(mLayer, this, mModel->GetMask(), mModel->GetUID(), dist);
	}
	return true;
}

//============================================================================================================
// Draw the object using the specified technique
//============================================================================================================

uint ModelInstance::OnDraw (const ITechnique* tech, bool insideOut)
{
	uint result(0);
	IGraphics* graphics = mCore->GetGraphics();

	if (mModel != 0)
	{
		if ((tech->GetMask() & mModel->GetMask()) != 0)
		{
			// Automatically normalize normals if the scale is not 1.0
			graphics->SetNormalize( Float::IsNotEqual(mAbsoluteScale, 1.0f) );

			// Set the model's world matrix so the rendered objects show up in their proper place
			graphics->SetModelMatrix( GetMatrix() );

			// Draw the model
			result += mModel->_Draw(graphics, tech);
		}

		if (mShowOutline)
		{
			// Draw the outline if requested
			static ITechnique* wireframe = graphics->GetTechnique("Wireframe");
			if (tech == wireframe) result += mModel->_DrawOutline(graphics, tech);
		}
	}
	return result;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void ModelInstance::OnSerializeTo (TreeNode& node) const
{
	if (mModel != 0)
	{
		node.AddChild( mModel->GetClassID(), mModel->GetName() );

		if (mCullBounds.IsValid())
		{
			TreeNode& child = node.AddChild("Culling Bounds");
			child.AddChild("Min", mCullBounds.GetMin());
			child.AddChild("Max", mCullBounds.GetMax());
		}
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool ModelInstance::OnSerializeFrom (const TreeNode& root)
{
	const String&	tag   = root.mTag;
	const Variable&	value = root.mValue;

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

		SetModel( model );
		return true;
	}
	else if ( tag == "Culling Bounds" )
	{
		Vector3f vMin, vMax;

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node	= root.mChildren[i];
			const String&	myTag	= node.mTag;
			const Variable& myVal	= node.mValue;

			if ( myTag == "Min" )
			{
				myVal >> vMin;
			}
			else if ( myTag == "Max" )
			{
				myVal >> vMax;
			}
		}

		mCullBounds.Set(vMin, vMax);
		return true;
	}
	return false;
}