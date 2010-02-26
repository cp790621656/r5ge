#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Recalculate the bounds
//============================================================================================================

void Billboard::OnUpdate()
{
	if (mIsDirty)
	{
		mAbsoluteBounds.Reset();
		mAbsoluteBounds.Include(mAbsolutePos, mAbsoluteScale);
	}
}

//============================================================================================================
// Add the billboard to the draw list
//============================================================================================================

bool Billboard::OnFill (FillParams& params)
{
	if (mTex != 0 && mTech != 0)
	{
		// If we're writing to depth, use the texture for grouping. If not -- don't. This is done so because
		// different billboards still need to blend correctly and the only way to do that is to sort all of
		// them together, which wouldn't be possible if they ended up in different groups.

		uint group = mTech->GetDepthWrite() ? mTex->GetUID() : 0;
		params.mDrawQueue.Add(mLayer, this, mTech->GetMask(), group, params.GetDist(mAbsolutePos));
	}
	return true;
}

//============================================================================================================
// Draw the billboard
//============================================================================================================

uint Billboard::OnDraw (const ITechnique* tech, bool insideOut)
{
	IGraphics* graphics = mCore->GetGraphics();
	Matrix43 mat (graphics->GetViewMatrix());
	mat.PreTranslate(mAbsolutePos);
	mat.ReplaceScaling(mAbsoluteScale);
	graphics->SetModelViewMatrix(mat);
	DrawBillboard();
	graphics->ResetModelViewMatrix();
	return 1;
}

//============================================================================================================
// Draws the actual billboard reusing the common buffers
//============================================================================================================

void Billboard::DrawBillboard()
{
	static float vertices[] =
	{
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
	};

	static float texcoords[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	// Set up active render states
	IGraphics* graphics = mCore->GetGraphics();
	graphics->SetADT();
	graphics->SetActiveShader(0);
	graphics->SetActiveColor(mColor);
	graphics->SetActiveMaterial(mTex);

	// Set the buffers
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,	  0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,	  0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,	  0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex,  0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight, 0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,  0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0, (Vector2f*)texcoords );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,	 (Vector2f*)vertices );

	// Draw the glow
	graphics->DrawVertices (IGraphics::Primitive::Quad, 4);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Billboard::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Color", mColor);

	TreeNode& tex = root.AddChild(ITexture::ClassID());
	if (mTex != 0) tex.mValue = mTex->GetName();

	TreeNode& tech = root.AddChild(ITechnique::ClassID());
	if (mTech != 0) tech.mValue = mTech->GetName();
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Billboard::OnSerializeFrom (const TreeNode& root)
{
	const String&	tag   = root.mTag;
	const Variable&	value = root.mValue;

	if (tag == "Color")
	{
		value >> mColor;
		return true;
	}
	else if (tag == ITexture::ClassID())
	{
		mTex = mCore->GetGraphics()->GetTexture( value.IsString() ? value.AsString() : value.GetString() );
		return true;
	}
	else if (tag == ITechnique::ClassID())
	{
		mTech = mCore->GetGraphics()->GetTechnique( value.IsString() ? value.AsString() : value.GetString() );
		return true;
	}
	return false;
}