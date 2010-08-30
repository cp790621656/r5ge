#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Directional Light's constructor
//============================================================================================================

PointLight::PointLight() :
	mAmbient	(0.0f),
	mDiffuse	(1.0f),
	mSpecular	(1.0f),
	mBrightness	(1.0f),
	mRange		(10.0f),
	mPower		(2.0f),
	mShader0	(0),
	mShader1	(0)
{
	mProperties.mType = ILight::Type::Point;
	_UpdateColors();
	_UpdateAtten();
}

//============================================================================================================
// Updates appropriate fields in 'mProperties'
//============================================================================================================

inline void PointLight::_UpdateColors()
{
	mProperties.mAmbient  = Color4f(mAmbient  * mBrightness, mBrightness);
	mProperties.mDiffuse  = Color4f(mDiffuse  * mBrightness, mRange * mAbsoluteScale.Average());
	mProperties.mSpecular = Color4f(mSpecular * mBrightness, mPower);
}

//============================================================================================================

inline void PointLight::_UpdateAtten()
{
	mProperties.mAtten.x = mRange * mAbsoluteScale.Average();
	mProperties.mAtten.y = mPower;
}

//============================================================================================================
// Changing light parameters
//============================================================================================================

void PointLight::SetBrightness (float val)
{
	if (mBrightness != val)
	{
		mBrightness = val;
		_UpdateColors();
	}
}

//============================================================================================================

void PointLight::SetRange (float val)
{
	if (mRange != val)
	{
		mRange = val;
		_UpdateAtten();
		_UpdateColors();
	}
}

//============================================================================================================

void PointLight::SetPower (float val)
{
	if (val < 1.0f) val = 1.0f;

	if (mPower != val)
	{
		mPower = val;
		_UpdateAtten();
		_UpdateColors();
	}
}

//============================================================================================================
// Update the light parameters
//============================================================================================================

void PointLight::OnUpdate()
{
	if (mIsDirty)
	{
		mRelativeBounds.Clear();
		mRelativeBounds.Set(Vector3f(-mRange), Vector3f(mRange));

		mProperties.mPos = mAbsolutePos;
		mProperties.mDir = mAbsoluteRot.GetForward();

		_UpdateAtten();
		_UpdateColors();
	}
}

//============================================================================================================
// Fill the renderable object and visible light lists
//============================================================================================================

bool PointLight::OnFill (FillParams& params)
{
	float range = mRange * mAbsoluteScale.Average();

	if ( (mProperties.mDiffuse.IsVisibleRGB() || mProperties.mAmbient.IsVisibleRGB()) && range > 0.0001f )
	{
		float dist = (params.mCamPos - mAbsolutePos).Magnitude() - range;
		params.mDrawQueue.Add(this, dist);
	}
	return true;
}

//============================================================================================================
// Draw the point light
//============================================================================================================

void PointLight::OnDrawLight (TemporaryStorage& storage, bool setStates)
{
	IVBO* vbo = storage.GetVBO(0);
	IVBO* ibo = storage.GetVBO(1);
	static uint indexCount (0);

	// Reset the modelview matrix
	mGraphics->ResetModelViewMatrix();

	// Activate initial states
	if (setStates)
	{
		mGraphics->SetActiveMaterial((ITexture*)0);
		mGraphics->SetActiveTexture(0, storage.GetDepth());
		mGraphics->SetActiveTexture(1, storage.GetNormal());
		mGraphics->SetActiveTexture(2, storage.GetAO());
		mGraphics->SetScreenProjection(false);

		if (!vbo->IsValid())
		{
			Array<Vector3f> vertices;
			Array<ushort> indices;
			Shape::Icosahedron(vertices, indices, 1);
			indexCount = indices.GetSize();

			vbo->Set(vertices, IVBO::Type::Vertex);
			ibo->Set(indices,  IVBO::Type::Index);
		}

		// Enable depth testing as point lights have a definite volume
		mGraphics->SetDepthTest(true);
		mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, vbo, 0, IGraphics::DataType::Float, 3, 12 );
	}

	// Just in case
	ASSERT(vbo->IsValid(), "Failed to create a VBO?");

	// Copy the light information as we'll be modifying it
	ILight properties (mProperties);

	// The range of the light is stored in the first attenuation parameter. The 6.5%
	// increase is there because the generated sphere goes up to (and never exceeds)
	// the radius of 1. However this means that the drawn triangles can actually be
	// closer as the sphere is never perfectly round. Thus we increase the radius by
	// this amount in order to avoid any visible edges when drawing the light. Note
	// that 6.5% is based on observation only. For icosahedrons of 2 iterations this
	// multiplier can be reduced down to 2%.

	float range (properties.mAtten.x * 1.065f);

	// Distance to the light source
	bool flip = properties.mPos.GetDistanceTo(mGraphics->GetCameraPosition()) <
		(range + mGraphics->GetCameraRange().x * 2.0f);

	// Start with the view matrix and apply the light's world transforms
	Matrix43 mat (mGraphics->GetViewMatrix());
	mat.PreTranslate(properties.mPos);
	mat.PreScale(range);

	// Set the matrix that will be used to transform this light and to draw it at the correct position
	mGraphics->SetModelViewMatrix(mat);

	// Reset the light's position as it will be transformed by the matrix we set above.
	// This is done in order to avoid an extra matrix switch, taking advantage of the
	// fact that OpenGL transforms light coordinates by the current ModelView matrix.
	properties.mPos = Vector3f();

	// First light activates the shader
	if (setStates)
	{
		if (mShader0 == 0)
		{
			mShader0 = mGraphics->GetShader("[R5] Light/Point");
			mShader1 = mGraphics->GetShader("[R5] Light/PointAO");
		}
		mGraphics->SetActiveShader( (storage.GetAO() == 0) ? mShader0 : mShader1);
	}

	// Activate the light at the matrix-transformed origin
	mGraphics->SetActiveLight(0, &properties);

	if (flip)
	{
		// The camera is inside the sphere -- draw the inner side, and only
		// on pixels that are closer to the camera than the light's range.

		mGraphics->SetCulling( IGraphics::Culling::Front );
		mGraphics->SetActiveDepthFunction( IGraphics::Condition::Greater );
		mGraphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, indexCount);
		mGraphics->SetActiveDepthFunction( IGraphics::Condition::Less );
		mGraphics->SetCulling(IGraphics::Culling::Back);
	}
	else
	{
		// Draw the light's sphere at the matrix-transformed position
		mGraphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, indexCount);
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void PointLight::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Ambient", mAmbient);
	root.AddChild("Diffuse", mDiffuse);
	root.AddChild("Specular", mSpecular);
	root.AddChild("Brightness", mBrightness);
	root.AddChild("Range", mRange);
	root.AddChild("Power", mPower);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool PointLight::OnSerializeFrom (const TreeNode& node)
{
	const String&	tag   = node.mTag;
	const Variable&	value = node.mValue;

	float val (0.0f);
	Color3f color;

	if		( tag == "Ambient"		&& value >> color ) SetAmbient(color);
	else if ( tag == "Diffuse" 		&& value >> color ) SetDiffuse(color);
	else if ( tag == "Specular"		&& value >> color ) SetSpecular(color);
	else if ( tag == "Brightness"	&& value >> val   )	SetBrightness(val);
	else if ( tag == "Range"		&& value >> val   )	SetRange(val);
	else if ( tag == "Power"		&& value >> val   )	SetPower(val);
	else return false;
	return true;
}