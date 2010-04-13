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
		Light::Register<DirectionalLight>();
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
// Add all directional light contribution
//============================================================================================================

void DirectionalLight::_Draw (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap)
{
	static IShader* dirShader0	 = graphics->GetShader("[R5] Light/Directional");
	static IShader* dirShader1	 = graphics->GetShader("[R5] Light/DirectionalAO");

	IShader* shader = (lightmap != 0) ? dirShader1 : dirShader0;

	// No depth test as directional light has no volume
	graphics->SetActiveTexture(2, lightmap);
	graphics->SetDepthTest(false);
	graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
	graphics->ResetModelViewMatrix();
	graphics->SetActiveShader(shader);

	// Run through all directional lights
	for (uint i = 0; i < lights.GetSize(); ++i)
	{
		graphics->SetActiveLight(i, lights[i].mLight);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
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