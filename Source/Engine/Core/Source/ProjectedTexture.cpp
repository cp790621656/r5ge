#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Set the shader and technique mask
//============================================================================================================

void ProjectedTexture::OnInit()
{
	mAddSubtract	= mGraphics->GetShader("[R5] Projected Texture: Add");
	mReplace		= mGraphics->GetShader("[R5] Projected Texture: Replace");
	mModulate		= mGraphics->GetShader("[R5] Projected Texture: Modulate");
	mMask			= mGraphics->GetTechnique("Projected Texture")->GetMask();
}

//============================================================================================================
// Update function should update the transformation matrix if coordinates have changed
//============================================================================================================

void ProjectedTexture::OnUpdate()
{
	if (mIsDirty)
	{
		mMatrix.SetToTransform(mAbsolutePos, mAbsoluteRot, mAbsoluteScale);

		mAbsoluteBounds.Clear();
		mAbsoluteBounds.Set(Vector3f(-1.0f), Vector3f(1.0f));
		mAbsoluteBounds.Transform(mAbsolutePos, mAbsoluteRot, mAbsoluteScale);
	}
}

//============================================================================================================
// Fill the renderable object and visible light lists
//============================================================================================================

bool ProjectedTexture::OnFill (FillParams& params)
{
	if (mTex != 0 && mAddSubtract != 0)
	{
		params.mDrawQueue.Add(mLayer, this, 0, mMask, mTex->GetUID(), 0.0f);
	}
	return true;
}

//============================================================================================================
// Draws the projected texture
//============================================================================================================

uint ProjectedTexture::OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut)
{
	const ITexture* depth = storage.GetDepth();
	if (depth == 0) return 0;

	IVBO* vbo = storage.GetVBO(2);
	IVBO* ibo = storage.GetVBO(3);

	if (!vbo->IsValid())
	{
		Array<Vector3f> vertices;
		Array<ushort> indices;
		Shape::Box(vertices, indices);
		vbo->Set(vertices, IVBO::Type::Vertex);
		ibo->Set(indices,  IVBO::Type::Index);
	}

	// Set the color and world matrix
	mGraphics->SetActiveColor(mColor);
	mGraphics->SetNormalize(false);
	mGraphics->SetBlending(mBlending);
	mGraphics->SetModelMatrix(mMatrix);

	IShader* shader (0);

	// Activate the shader
	if (mBlending == ITechnique::Blending::Add || mBlending == ITechnique::Blending::Subtract)
	{
		mGraphics->SetActiveShader(shader = mAddSubtract);
	}
	else if (mBlending == ITechnique::Blending::Modulate)
	{
		mGraphics->SetActiveShader(shader = mModulate);
	}
	else
	{
		mGraphics->SetActiveShader(shader = mReplace);
	}

	// It's faster to invert a 4x3 matrix then multiply it by a cached inverted 4x4 matrix
	// than it is to invert a full 4x4 matrix every time a projected texture is drawn.
	const Matrix44& invProj = mGraphics->GetInverseProjMatrix();
	Matrix43 invView (mMatrix);
	invView *= mGraphics->GetViewMatrix();
	invView.Invert();
	Matrix44 invMVP (invProj);
	invMVP *= invView;

	// Update the shader uniforms
	shader->SetUniform("g_mat", invMVP);
	shader->SetUniform("g_color", mColor);

	// Activate the textures
	mGraphics->SetActiveTexture(0, depth);
	mGraphics->SetActiveTexture(1, mTex);
	mGraphics->SetActiveTexture(2, 0);
	mGraphics->SetActiveTexture(3, 0);
	mGraphics->SetActiveTexture(4, 0);

	// See if the camera intersects with the bounding box of the projected texture
	bool invert = mAbsoluteBounds.Intersects(mGraphics->GetCameraNearBounds());

	if (invert)
	{
		// If the camera is inside the bounds, switch to reverse culling and depth testing
		mGraphics->SetCulling( insideOut ? IGraphics::Culling::Back : IGraphics::Culling::Front );
		mGraphics->SetActiveDepthFunction( IGraphics::Condition::Greater );
	}
	else
	{
		// If the camera is outside of the bounds, use normal culling and depth testing
		mGraphics->SetCulling( insideOut ? IGraphics::Culling::Front : IGraphics::Culling::Back );
		mGraphics->SetActiveDepthFunction( IGraphics::Condition::Less );
	}

	// Disable all unused buffers, bind the position
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,		0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,		0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,
		vbo, 0, IGraphics::DataType::Float, 3, 12 );

	// Draw the projected texture
	mGraphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, ibo->GetSize() / sizeof(ushort));

	// Restore the depth function
	if (invert)
	{
		mGraphics->SetCulling( insideOut ? IGraphics::Culling::Front : IGraphics::Culling::Back );
		mGraphics->SetActiveDepthFunction( IGraphics::Condition::Less );
	}
	return 1;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void ProjectedTexture::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Color", mColor);
	if (mTex != 0) node.AddChild("Texture", mTex->GetName());

	const char* blending = "Replace";

	if		(mBlending == ITechnique::Blending::Add)		blending = "Add";
	else if	(mBlending == ITechnique::Blending::Modulate)	blending = "Modulate";
	else if (mBlending == ITechnique::Blending::Subtract)	blending = "Subtract";

	node.AddChild("Blending", blending);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool ProjectedTexture::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Color")
	{
		node.mValue >> mColor;
		return true;
	}
	else if (node.mTag == "Texture")
	{
		SetTexture( mGraphics->GetTexture(node.mValue.AsString()) );
		return true;
	}
	else if (node.mTag == "Blending")
	{
		const String& s = node.mValue.AsString();
		if		(s == "Add")		mBlending = ITechnique::Blending::Add;
		else if	(s == "Modulate")	mBlending = ITechnique::Blending::Modulate;
		else if (s == "Subtract")	mBlending = ITechnique::Blending::Subtract;
		else						mBlending = ITechnique::Blending::Replace;
		return true;
	}
	return false;
}