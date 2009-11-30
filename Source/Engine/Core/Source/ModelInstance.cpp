#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Debugging section used to show the actual bounding box itself
//============================================================================================================

void GetBoundingBoxLines (Array<Vector3f>& v, const Vector3f& min, const Vector3f& max)
{
	// Bottom X
	v.Expand().Set(min.x, min.y, min.z);
	v.Expand().Set(max.x, min.y, min.z);
	v.Expand().Set(min.x, max.y, min.z);
	v.Expand().Set(max.x, max.y, min.z);
	
	// Top X
	v.Expand().Set(min.x, min.y, max.z);
	v.Expand().Set(max.x, min.y, max.z);
	v.Expand().Set(min.x, max.y, max.z);
	v.Expand().Set(max.x, max.y, max.z);

	// Bottom Y
	v.Expand().Set(min.x, min.y, min.z);
	v.Expand().Set(min.x, max.y, min.z);
	v.Expand().Set(max.x, min.y, min.z);
	v.Expand().Set(max.x, max.y, min.z);

	// Top Y
	v.Expand().Set(min.x, min.y, max.z);
	v.Expand().Set(min.x, max.y, max.z);
	v.Expand().Set(max.x, min.y, max.z);
	v.Expand().Set(max.x, max.y, max.z);

	// Bottom Z
	v.Expand().Set(min.x, min.y, min.z);
	v.Expand().Set(min.x, min.y, max.z);
	v.Expand().Set(max.x, min.y, min.z);
	v.Expand().Set(max.x, min.y, max.z);

	// Top Z
	v.Expand().Set(min.x, max.y, min.z);
	v.Expand().Set(min.x, max.y, max.z);
	v.Expand().Set(max.x, max.y, min.z);
	v.Expand().Set(max.x, max.y, max.z);
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
		mMatrix.SetToTransform( mAbsolutePos, mAbsoluteRot, mAbsoluteScale );

		mBounds = mCullBounds;

		if (mModel != 0)
		{
			mBounds.Include( mModel->GetBounds() );
		}

		mBounds.Transform( mAbsolutePos, mAbsoluteRot, mAbsoluteScale );
	}
}

//============================================================================================================
// Culls the object based on the viewing frustum
//============================================================================================================

Object::CullResult ModelInstance::OnCull (CullParams& params, bool isParentVisible, bool render)
{
	// Only draw the model if the parent is visible and if the bounds are within the frustum
	if (isParentVisible && (mModel != 0 && params.mFrustum.IsVisible(mBounds)) )
	{
		if (render)
		{
			Drawable& obj	 = params.mObjects.Expand();
			obj.mObject		 = this;
			obj.mMask		 = mModel->GetMask();
			obj.mLayer		 = mModel->GetLayer();
			obj.mGroup		 = mModel;
			obj.mDistSquared = (mAbsolutePos - params.mCamPos).Dot();
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Draw the object using the specified technique
//============================================================================================================

uint ModelInstance::OnDraw (const ITechnique* tech, bool insideOut)
{
	uint triangleCount(0);
	IGraphics* graphics = mCore->GetGraphics();

	// Automatically normalize normals if the scale is not 1.0
	graphics->SetNormalize( Float::IsNotEqual(mAbsoluteScale, 1.0f) );

	// Set the model's world matrix so the rendered objects show up in the proper place
	graphics->SetWorldMatrix(mMatrix);

	if (mModel != 0)
	{
		// Draw the model
		triangleCount += mModel->_Draw(graphics, tech);

		if (mShowOutline)
		{
			// Draw the outline if requested
			triangleCount += mModel->_DrawOutline(graphics, tech);
			triangleCount += _DrawOutline(graphics, tech);
		}
	}
	else if (mShowOutline)
	{
		// Draw only the outline
		triangleCount += _DrawOutline(graphics, tech);
	}
	return triangleCount;
}

//============================================================================================================
// Selects the closest object to the given position if the position is within the object's bounds
//============================================================================================================

bool ModelInstance::OnSelect (const Vector3f& pos, ObjectPtr& ptr, float& radius)
{
	if (mBounds.Contains(pos))
	{
		const Vector3f& center = mBounds.GetCenter();
		float distance = (center - pos).Dot();

		if (distance < radius)
		{
			radius = distance;
			ptr = this;
		}
	}
	return true;
}

//============================================================================================================
// Draws the outline of the bounding box
//============================================================================================================

uint ModelInstance::_DrawOutline (IGraphics* graphics, const ITechnique* tech)
{
	uint triangleCount (0);

	// If the model's outline is requested, draw it now
	if (tech->GetColorWrite())
	{
		graphics->ResetWorldMatrix();

		Array<Vector3f> v;
		GetBoundingBoxLines(v, mBounds.GetMin(), mBounds.GetMax());

		float factor = Float::Abs(Float::Cos(4.0f * Time::GetTime()));
		bool stencil = graphics->GetStencilTest();

		graphics->SetLighting   (false);
		graphics->SetDepthWrite (true);
		graphics->SetDepthTest	(true);
		graphics->SetActiveTechnique(0);
		graphics->SetActiveMaterial (0);
		graphics->SetActiveColor( Color4f(factor, 1.0f - factor, 0, 1) );

		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Normal,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Color,		0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord0,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord1,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Position,	v);

		triangleCount += graphics->DrawVertices( IGraphics::Primitive::Line, v.GetSize() );

		// Restore the active technique
		graphics->SetStencilTest(stencil);
		graphics->SetActiveColor( Color4f(1, 1, 1, 1) );
		graphics->SetActiveTechnique(tech);
	}
	return triangleCount;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void ModelInstance::OnSerializeTo (TreeNode& root) const
{
	if (mModel != 0)
	{
		root.AddChild( mModel->GetClassID(), mModel->GetName() );

		if (mCullBounds.IsValid())
		{
			TreeNode& node = root.AddChild("Culling Bounds");
			node.AddChild("Min", mCullBounds.GetMin());
			node.AddChild("Max", mCullBounds.GetMax());
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