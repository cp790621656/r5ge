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
		params.mDrawQueue.Add(mLayer, this, mModel->GetMask(), mModel, dist);
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

	// Automatically normalize normals if the scale is not 1.0
	graphics->SetNormalize( Float::IsNotEqual(mAbsoluteScale, 1.0f) );

	// Set the model's world matrix so the rendered objects show up in their proper place
	graphics->SetModelMatrix( GetMatrix() );

	if (mModel != 0)
	{
		// Draw the model
		result += mModel->_Draw(graphics, tech);

		if (mShowOutline)
		{
			// Draw the outline if requested
			result += mModel->_DrawOutline(graphics, tech);
			result += _DrawOutline(graphics, tech);
		}
	}
	else if (mShowOutline)
	{
		// Draw only the outline
		result += _DrawOutline(graphics, tech);
	}
	return result;
}

//============================================================================================================
// Draws the outline of the bounding box
//============================================================================================================

uint ModelInstance::_DrawOutline (IGraphics* graphics, const ITechnique* tech)
{
	uint result(0);

	// If the model's outline is requested, draw it now
	if (tech->GetColorWrite())
	{
		graphics->ResetModelMatrix();

		Array<Vector3f> v;
		const Bounds& complete = GetCompleteBounds();
		GetBoundingBoxLines(v, complete.GetMin(), complete.GetMax());

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

		result += graphics->DrawVertices( IGraphics::Primitive::Line, v.GetSize() );

		// Restore the active technique
		graphics->SetStencilTest(stencil);
		graphics->SetActiveColor( Color4f(1.0f) );
		graphics->SetActiveTechnique(tech);
	}
	return result;
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
		if (mShowOutline) root.AddChild("Show Outline", mShowOutline);
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
	else if (tag == "Show Outline")
	{
		mShowOutline = value.IsBool() ? value.AsBool() : true;
	}
	return false;
}