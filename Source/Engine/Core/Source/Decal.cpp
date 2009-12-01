#include "../Include/_All.h"
using namespace R5;

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
		mMatrix.SetToTransform( mAbsolutePos, mAbsoluteRot, mAbsoluteScale );
	}
}

//============================================================================================================
// Cull the object based on the viewing frustum
//============================================================================================================

Object::CullResult Decal::OnCull (CullParams& params, bool isParentVisible, bool render)
{
	// Save the mask as it doesn't change
	static uint mask = mCore->GetGraphics()->GetTechnique("Decal")->GetMask();

	if (mShader != 0 && params.mFrustum.IsVisible(mAbsolutePos, mAbsoluteScale * 1.415f))
	{
		if (render)
		{
			// Add a new drawable entry
			Drawable& obj = params.mObjects.Expand();
			obj.mObject = this;
			obj.mMask = mask;

			// Layer -2 is for terrains, '-1' is for decals.
			obj.mLayer = -1;

			// Group by the last texture. Shaders are more likely to be shared.
			obj.mGroup = mTextures.IsValid() ? mTextures.Back() : 0;

			// Distance is not used so that decals don't pop in and out of each other as the camera
			// is moving. It's best if they stay in the same exact draw order for better or for worse.
			obj.mDistSquared = 0.0f;
			return CullResult(true, true, mask);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Draws the decal
//============================================================================================================

uint Decal::OnDraw (const ITechnique* tech, bool insideOut)
{
	static IVBO* vbo = 0;
	static IVBO* ibo = 0;
	static uint indexCount = 0;

	IGraphics* graphics = mCore->GetGraphics();

	if (vbo == 0)
	{
		vbo = graphics->CreateVBO();
		ibo = graphics->CreateVBO();

		Array<Vector3f> vertices;
		Array<ushort> indices;
		Shape::Box(vertices, indices);
		indexCount = indices.GetSize();

		vbo->Set(vertices, IVBO::Type::Vertex);
		ibo->Set(indices,  IVBO::Type::Index);
	}

	// Update the values used by the shader
	const Matrix43& mat = graphics->GetViewMatrix();
	g_pos.xyz()	= mAbsolutePos * mat;
	g_pos.w		= mAbsoluteScale;
	g_forward	= mAbsoluteRot.GetForward() % mat;
	g_right		= mAbsoluteRot.GetRight() % mat;
	g_up		= mAbsoluteRot.GetUp() % mat;

	// Finish all draw operations
	graphics->Flush();

	// Set the color and world matrix
	graphics->SetActiveColor(mColor);
	graphics->SetNormalize(false);
	graphics->SetWorldMatrix(mMatrix);

	// Activate the shader, force-updating the uniforms
	graphics->SetActiveShader(mShader, true);

	// Bind all textures
	for (uint i = 0; i < mTextures.GetSize(); ++i)
		graphics->SetActiveTexture(i, mTextures[i]);

	// Distance from the center to the farthest corner of the box before it starts getting clipped
	float range = mAbsoluteScale * 1.415f + graphics->GetCameraRange().x * 2.0f;

	if ( mAbsolutePos.GetDistanceTo(graphics->GetCameraPosition()) > range )
	{
		// If the camera is outside of the sphere, use normal culling and depth testing
		graphics->SetCulling( IGraphics::Culling::Back );
		graphics->SetActiveDepthFunction( IGraphics::Condition::Less );
	}
	else
	{
		// If the camera is inside the sphere, switch to reverse culling and depth testing
		graphics->SetCulling( IGraphics::Culling::Front );
		graphics->SetActiveDepthFunction( IGraphics::Condition::Greater );
	}

	// Disable all unused buffers, bind the position
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,
		vbo, 0, IGraphics::DataType::Float, 3, 12 );

	// Draw the decal
	return graphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, indexCount);
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

bool Decal::OnSerializeFrom (const TreeNode& root)
{
	IGraphics* graphics = mCore->GetGraphics();
	if (graphics == 0) return false;

	const String&	tag   = root.mTag;
	const Variable&	value = root.mValue;

	if (tag == "Shader")
	{
		SetShader( graphics->GetShader(value.IsString() ? value.AsString() : value.GetString()) );
	}
	else if (tag == "Color")
	{
		if (root.mValue.IsColor4ub())
		{
			SetColor(root.mValue.AsColor4ub());
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
				const ITexture* tex = graphics->GetTexture(list[i]);
				if (tex != 0) mTextures.Expand() = tex;
			}
		}
	}
	else return false;
	return true;
}