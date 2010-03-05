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
	mLight.mSubtype = HashKey(ClassID());

	_UpdateColors();

	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;
		Deferred::RegisterLight(mLight.mSubtype, Deferred::DrawDirectionalLights);
	}
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

bool DirectionalLight::OnSerializeFrom (const TreeNode& node)
{
	const String&	tag   = node.mTag;
	const Variable&	value = node.mValue;

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

void DirectionalLight::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Ambient",	mAmbient);
	node.AddChild("Diffuse",	mDiffuse);
	node.AddChild("Specular",	mSpecular);
	node.AddChild("Brightness", mBrightness);
}