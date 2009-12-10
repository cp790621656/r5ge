#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Directional Light's constructor
//============================================================================================================

DirectionalLight::DirectionalLight() :
	mAmbient	(0.0f),
	mDiffuse	(1.0f),
	mSpecular	(1.0f),
	mBrightness	(1.0f)
{
	mLight.mType = Light::Type::Directional;
	_UpdateColors();
}

//============================================================================================================
// Update the 'mParams' light colors
//============================================================================================================

void DirectionalLight::_UpdateColors()
{
	mLight.mAmbient  = Color4f(mAmbient  * mBrightness, mBrightness);
	mLight.mDiffuse  = Color4f(mDiffuse  * mBrightness, 1.0f);
	mLight.mSpecular = Color4f(mSpecular * mBrightness, 1.0f);
}

//============================================================================================================
// Update the light parameters
//============================================================================================================

void DirectionalLight::OnUpdate()
{
	if (mIsDirty)
	{
		mLight.mPos = mAbsolutePos;
		mLight.mDir = mAbsoluteRot.GetForward();
	}
}

//============================================================================================================
// Fill the renderable object and visible light lists
//============================================================================================================

bool DirectionalLight::OnFill (FillParams& params)
{
	if (mLight.mDiffuse.IsVisibleRGB() || mLight.mAmbient.IsVisibleRGB())
	{
		params.mDrawQueue.Add(&mLight);
	}
	return true;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool DirectionalLight::OnSerializeFrom (const TreeNode& root)
{
	const String&	tag   = root.mTag;
	const Variable&	value = root.mValue;

	float f;
	Color3f color;

	if		( tag == "Ambient"		&& value >> color ) SetAmbient(color);
	else if ( tag == "Diffuse" 		&& value >> color ) SetDiffuse(color);
	else if ( tag == "Specular"		&& value >> color ) SetSpecular(color);
	else if ( tag == "Brightness"	&& value >> f	  )	SetBrightness(f);
	else return false;
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void DirectionalLight::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Ambient",	mAmbient);
	root.AddChild("Diffuse",	mDiffuse);
	root.AddChild("Specular",	mSpecular);
	root.AddChild("Brightness", mBrightness);
}