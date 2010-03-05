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
		Deferred::RegisterLight(mLight.mSubtype, Deferred::DrawPointLights);
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