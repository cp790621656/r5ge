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
// Global values needed by the shader
//============================================================================================================

Quaternion	g_pos;
Vector3f	g_forward;
Vector3f	g_right;
Vector3f	g_up;

//============================================================================================================
// Shader uniform update callback functions
//============================================================================================================

void OnSetPos		(const String& name, Uniform& uni) { uni = g_pos;		}
void OnSetForward	(const String& name, Uniform& uni) { uni = g_forward;	}
void OnSetRight		(const String& name, Uniform& uni) { uni = g_right;		}
void OnSetUp		(const String& name, Uniform& uni) { uni = g_up;		}

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
// Changing the shader means having to re-register uniforms used by this shader
//============================================================================================================

void Decal::SetShader (IShader* shader)
{
	mShader = shader;

	if (mShader != 0)
	{
		mShader->RegisterUniform("g_pos",	  OnSetPos);
		mShader->RegisterUniform("g_forward", OnSetForward);
		mShader->RegisterUniform("g_right",	  OnSetRight);
		mShader->RegisterUniform("g_up",	  OnSetUp);
	}
}

//============================================================================================================
// Update function should update the transformation matrix if coordinates have changed
//============================================================================================================

void Decal::OnUpdate()
{
	if (mIsDirty)
	{
		// Recalculate absolute bounds directly as it's faster than having to transform relative bounds.
		// 1.732f multiplication is here because we draw a cube, but its corners are sqrt(3) farther away
		// than the sides. In order to cull it properly we treat it as a maximum range sphere instead.
		mAbsoluteBounds.Clear();
		mAbsoluteBounds.Include(mAbsolutePos, mAbsoluteScale * 1.732f);

		// Transform matrix uses calculated absolute values
		mMatrix.SetToTransform(mAbsolutePos, mAbsoluteRot, mAbsoluteScale);
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

	// Update the values used by the shader
	const Matrix43& mat = mGraphics->GetViewMatrix();
	g_pos.xyz()	= mAbsolutePos * mat;
	g_pos.w		= mAbsoluteScale;
	g_forward	= mAbsoluteRot.GetForward() % mat;
	g_right		= mAbsoluteRot.GetRight() % mat;
	g_up		= mAbsoluteRot.GetUp() % mat;

	// Finish all draw operations
	mGraphics->Flush();

	// Set the color and world matrix
	mGraphics->SetActiveColor(mColor);
	mGraphics->SetNormalize(false);
	mGraphics->SetModelMatrix(mMatrix);

	// Activate the shader, force-updating the uniforms
	mGraphics->SetActiveShader(mShader, true);

	// ATI cards seem to clamp gl_FrontMaterial.diffuse in 0-1 range
	if (mShader != 0) mShader->SetUniform("g_color", mColor);

	// Activate the 4 mandatory textures
	mGraphics->SetActiveTexture(0, depth);
	mGraphics->SetActiveTexture(1, normal);
	mGraphics->SetActiveTexture(2, diffuse);
	mGraphics->SetActiveTexture(3, specular);

	// Activate optional textures
	for (uint i = 0; i < mTextures.GetSize(); ++i)
		mGraphics->SetActiveTexture(i+4, mTextures[i]);

	// Distance from the center to the farthest corner of the box before it starts getting clipped
	float range = mAbsoluteScale * 1.732f + mGraphics->GetCameraRange().x;

	// Invert the depth testing and culling if the camera is inside the box
	bool invert = mAbsolutePos.GetDistanceTo(mGraphics->GetCameraPosition()) < range;

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