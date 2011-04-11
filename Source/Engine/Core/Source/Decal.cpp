#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the default drawing layer that will be used by decals
//============================================================================================================

byte g_decalLayer = 5;

void Decal::SetDefaultLayer (byte layer)
{
	g_decalLayer = layer & 31;
}

//============================================================================================================
// Use the default decal layer by default
//============================================================================================================

Decal::Decal() : mShader(0), mColor(1.0f)
{
	mLayer = g_decalLayer;
	mCalcAbsBounds = false;
}

//============================================================================================================
// Set the mask
//============================================================================================================

void Decal::OnInit() { mMask = mGraphics->GetTechnique("Decal")->GetMask(); }

//============================================================================================================
// Changes the shader to the specified one
//============================================================================================================

void Decal::SetShader (const String& shader)
{
	IGraphics* graphics = GetGraphics();
	if (graphics != 0) SetShader( graphics->GetShader(shader) );
}

//============================================================================================================
// Update function should update the transformation matrix if coordinates have changed
//============================================================================================================

void Decal::OnUpdate()
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

bool Decal::OnFill (FillParams& params)
{
	if (mShader != 0)
	{
		uint group (0);

		if (mTextures.IsValid() && mTextures.Back() != 0)
		{
			group = mTextures.Back()->GetUID();
		}
		else if (mShader != 0)
		{
			group = mShader->GetUID();
		}
		params.mDrawQueue.Add(mLayer, this, 0, mMask, group, 0.0f);
	}
	return true;
}

//============================================================================================================
// Draws the decal
//============================================================================================================

uint Decal::OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut)
{
	if (mShader == 0) return 0;

	IVBO* vbo = storage.GetVBO(2);
	IVBO* ibo = storage.GetVBO(3);

	const ITexture* depth	 = storage.GetRenderTexture(0);
	const ITexture* normal	 = storage.GetRenderTexture(1);
	const ITexture* diffuse	 = storage.GetRenderTexture(2);
	const ITexture* specular = storage.GetRenderTexture(3);

	// These textures are only available during the deferred process -- and so are decals
	if (depth == 0 || normal == 0 || diffuse == 0 || specular == 0) return 0;

	if (!vbo->IsValid())
	{
		Array<Vector3f> vertices;
		Array<ushort> indices;
		Shape::Box(vertices, indices);

		vbo->Set(vertices, IVBO::Type::Vertex);
		ibo->Set(indices,  IVBO::Type::Index);
	}

	// Finish all draw operations
	mGraphics->Flush();

	// Set the color and world matrix
	mGraphics->SetActiveColor(mColor);
	mGraphics->SetNormalize(false);
	mGraphics->SetModelMatrix(mMatrix);

	// Activate the shader, force-updating the uniforms
	mGraphics->SetActiveShader(mShader);

	// It's faster to invert a 4x3 matrix then multiply it by a cached inverted 4x4 matrix
	// than it is to invert a full 4x4 matrix every time a projected texture is drawn.
	const Matrix44& invProj = mGraphics->GetInverseProjMatrix();
	Matrix43 invView (mMatrix);
	invView *= mGraphics->GetViewMatrix();
	invView.Invert();
	Matrix44 invMVP (invProj);
	invMVP *= invView;

	// Update the shader uniforms
	const Matrix43& mat = mGraphics->GetViewMatrix();
	mShader->SetUniform("g_mat",	 invMVP);
	mShader->SetUniform("g_forward", mAbsoluteRot.GetForward() % mat);
	mShader->SetUniform("g_right",	 mAbsoluteRot.GetRight() % mat);
	mShader->SetUniform("g_up",		 mAbsoluteRot.GetUp() % mat);
	mShader->SetUniform("g_color",	 mColor);

	// ATI cards seem to clamp gl_FrontMaterial.diffuse in 0-1 range
	mShader->SetUniform("g_color", mColor);

	// Activate the 4 mandatory textures
	mGraphics->SetActiveTexture(0, depth);
	mGraphics->SetActiveTexture(1, normal);
	mGraphics->SetActiveTexture(2, diffuse);
	mGraphics->SetActiveTexture(3, specular);

	// Activate optional textures
	for (uint i = 0; i < mTextures.GetSize(); ++i)
		mGraphics->SetActiveTexture(i+4, mTextures[i]);

	// See if the camera intersects with the bounding box of the projected texture
	bool invert = mAbsoluteBounds.Intersects(mGraphics->GetCameraNearBounds());

	if (invert)
	{
		// If the camera is inside the sphere, switch to reverse culling and depth testing
		mGraphics->SetCulling( insideOut ? IGraphics::Culling::Back : IGraphics::Culling::Front );
		mGraphics->SetActiveDepthFunction( IGraphics::Condition::Greater );
	}
	else
	{
		// If the camera is outside of the sphere, use normal culling and depth testing
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

	// Draw the decal
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

void Decal::OnSerializeTo (TreeNode& root) const
{
	if (mShader	!= 0) root.AddChild("Shader", mShader->GetName());

	root.AddChild("Color", mColor);

	if (mTextures.IsValid())
	{
		TreeNode& node = root.AddChild("Textures");
		Array<String>& list = node.mValue.ToStringArray();

		for (uint i = 0; i < mTextures.GetSize(); ++i)
		{
			const ITexture* tex = mTextures[i];
			if (tex != 0) list.Expand() = tex->GetName();
		}
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Decal::OnSerializeFrom (const TreeNode& node)
{
	IGraphics* graphics = mCore->GetGraphics();
	if (graphics == 0) return false;

	const String&	tag   = node.mTag;
	const Variable&	value = node.mValue;

	if (tag == "Shader")
	{
		SetShader( graphics->GetShader(value.AsString()) );
	}
	else if (tag == "Color")
	{
		if (value.IsColor4f())
		{
			SetColor(value.AsColor4f());
		}
		else if (value.IsColor4ub())
		{
			SetColor(value.AsColor4ub());
		}
	}
	else if (tag == "Textures")
	{
		mTextures.Clear();

		if (value.IsStringArray())
		{
			const Array<String>& list = value.AsStringArray();

			for (uint i = 0; i < list.GetSize(); ++i)
			{
				// Skip legacy functionality
				if (!list[i].BeginsWith("[Generated]"))
				{
					const ITexture* tex = graphics->GetTexture(list[i]);
					if (tex != 0) mTextures.Expand() = tex;
				}
			}
		}
	}
	else return false;
	return true;
}