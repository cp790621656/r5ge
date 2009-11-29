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
	_UpdateColors();
	_UpdateAtten();
}

//============================================================================================================
// Updates appropriate fields in 'mParams'
//============================================================================================================

inline void PointLight::_UpdateColors()
{
	mParams.mAmbient  = Color4f(mAmbient  * mBrightness, mBrightness);
	mParams.mDiffuse  = Color4f(mDiffuse  * mBrightness, mRange * mAbsoluteScale);
	mParams.mSpecular = Color4f(mSpecular * mBrightness, mPower);
}

//============================================================================================================

inline void PointLight::_UpdateAtten()
{
	mParams.mAtten.x = mRange * mAbsoluteScale;
	mParams.mAtten.y = mPower;
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
	mParams.mPos = mAbsolutePos;
	mParams.mDir = mAbsoluteRot.GetForward();
	_UpdateAtten();
	_UpdateColors();
}

//============================================================================================================
// Cull the object based on the viewing frustum
//============================================================================================================

Object::CullResult PointLight::OnCull (CullParams& params, bool isParentVisible, bool render)
{
	float range = mRange * mAbsoluteScale;

	if ( isParentVisible && (mParams.IsVisible() && range > 0.0001f) &&
		 params.mFrustum.IsVisible(mAbsolutePos, range) )
	{
		params.mLights.Expand() = &mParams;
		return true;
	}
	return false;
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