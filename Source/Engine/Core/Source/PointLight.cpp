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
	mPower		(2.0f)
{
	mLight.mType = Light::Type::Point;
	mLight.mSubtype = HashKey(ClassID());

	_UpdateColors();
	_UpdateAtten();

	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;
		Light::Register<PointLight>();
	}
}

//============================================================================================================
// Updates appropriate fields in 'mParams'
//============================================================================================================

inline void PointLight::_UpdateColors()
{
	mLight.mAmbient  = Color4f(mAmbient  * mBrightness, mBrightness);
	mLight.mDiffuse  = Color4f(mDiffuse  * mBrightness, mRange * mAbsoluteScale);
	mLight.mSpecular = Color4f(mSpecular * mBrightness, mPower);
}

//============================================================================================================

inline void PointLight::_UpdateAtten()
{
	mLight.mAtten.x = mRange * mAbsoluteScale;
	mLight.mAtten.y = mPower;
}

//============================================================================================================
// Add all point light contribution
//============================================================================================================

void PointLight::_Draw (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap)
{
	static IShader* dirShader0	 = graphics->GetShader("[R5] Light/Point");
	static IShader* dirShader1	 = graphics->GetShader("[R5] Light/PointAO");

	IShader* shader = (lightmap != 0) ? dirShader1 : dirShader0;

	graphics->SetActiveTexture(2, lightmap);
	graphics->SetActiveProjection( IGraphics::Projection::Perspective );

	float nearClip = graphics->GetCameraRange().x;
	const Vector3f& camPos = graphics->GetCameraPosition();
	
	static IVBO* vbo = 0;
	static IVBO* ibo = 0;
	static uint indexCount = 0;

	if (vbo == 0)
	{
		vbo = graphics->CreateVBO();
		ibo = graphics->CreateVBO();

		Array<Vector3f> vertices;
		Array<ushort> indices;
		Shape::Icosahedron(vertices, indices, 1);
		indexCount = indices.GetSize();

		vbo->Set(vertices, IVBO::Type::Vertex);
		ibo->Set(indices,  IVBO::Type::Index);
	}

	// Enable depth testing as point lights have a definite volume
	graphics->SetDepthTest(true);
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, vbo, 0, IGraphics::DataType::Float, 3, 12 );

	// Disable all active lights except the first
	for (uint b = lights.GetSize(); b > 1; )
		graphics->SetActiveLight(--b, 0);

	// Save the view matrix as it won't be changing
	const Matrix43& view = graphics->GetViewMatrix();
	Matrix43 mat;

	// Run through all point lights
	for (uint i = 0; i < lights.GetSize(); ++i)
	{
		const Light::Entry& entry = lights[i];

		// Copy the light information as we'll be modifying it
		Light light (*entry.mLight);

		// The range of the light is stored in the first attenuation parameter. The 6.5%
		// increase is there because the generated sphere goes up to (and never exceeds)
		// the radius of 1. However this means that the drawn triangles can actually be
		// closer as the sphere is never perfectly round. Thus we increase the radius by
		// this amount in order to avoid any visible edges when drawing the light. Note
		// that 6.5% is based on observation only. For icosahedrons of 2 iterations this
		// multiplier can be reduced down to 2%.

		float range (light.mAtten.x * 1.065f);

		// Distance to the light source
		float dist (light.mPos.GetDistanceTo(camPos) > (range + nearClip * 2.0f));

		// Start with the view matrix and apply the light's world transforms
		mat = view;
		mat.PreTranslate(light.mPos);
		mat.PreScale(range);

		// Set the matrix that will be used to transform this light and to draw it at the correct position
		graphics->SetModelViewMatrix(mat);

		// Reset the light's position as it will be transformed by the matrix we set above.
		// This is done in order to avoid an extra matrix switch, taking advantage of the
		// fact that OpenGL transforms light coordinates by the current ModelView matrix.
		light.mPos = Vector3f();

		// First light activates the shader
		if (i == 0) graphics->SetActiveShader(shader);

		// Activate the light at the matrix-transformed origin
		graphics->SetActiveLight(0, &light);

		if (dist)
		{
			// The camera is outside the sphere -- regular rendering approach
			graphics->SetCulling( IGraphics::Culling::Back );
			graphics->SetActiveDepthFunction( IGraphics::Condition::Less );
		}
		else
		{
			// The camera is inside the sphere -- draw the inner side, and only
			// on pixels that are closer to the camera than the light's range.

			graphics->SetCulling( IGraphics::Culling::Front );
			graphics->SetActiveDepthFunction( IGraphics::Condition::Greater );
		}

		// Draw the light's sphere at the matrix-transformed position
		graphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, indexCount);
	}

	// Restore important states
	graphics->SetActiveDepthFunction( IGraphics::Condition::Less );
	graphics->SetCulling(IGraphics::Culling::Back);
	graphics->ResetModelViewMatrix();
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
		mRelativeBounds.Reset();
		mRelativeBounds.Include(Vector3f(), mRange);

		mLight.mPos = mAbsolutePos;
		mLight.mDir = mAbsoluteRot.GetForward();

		_UpdateAtten();
		_UpdateColors();
	}
}

//============================================================================================================
// Fill the renderable object and visible light lists
//============================================================================================================

bool PointLight::OnFill (FillParams& params)
{
	float range = mRange * mAbsoluteScale;

	if ( (mLight.mDiffuse.IsVisibleRGB() || mLight.mAmbient.IsVisibleRGB()) && range > 0.0001f )
	{
		params.mDrawQueue.Add(&mLight);
	}
	return true;
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

bool PointLight::OnSerializeFrom (const TreeNode& root)
{
	const String&	tag   = root.mTag;
	const Variable&	value = root.mValue;

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