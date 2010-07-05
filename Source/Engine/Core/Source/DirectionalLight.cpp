#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Directional Light's constructor
//============================================================================================================

DirectionalLight::DirectionalLight() :
	mAmbient	(0.0f),
	mDiffuse	(1.0f),
	mSpecular	(1.0f),
	mBrightness	(1.0f),
	mShader00	(0),
	mShader01	(0)
{
	mProperties.mType = ILight::Type::Directional;
	_UpdateColors();
}

//============================================================================================================
// Update the 'mParams' light colors
//============================================================================================================

void DirectionalLight::_UpdateColors()
{
	mProperties.mAmbient  = Color4f(mAmbient  * mBrightness, mBrightness);
	mProperties.mDiffuse  = Color4f(mDiffuse  * mBrightness, 1.0f);
	mProperties.mSpecular = Color4f(mSpecular * mBrightness, 1.0f);
}

//============================================================================================================
// Update the light parameters
//============================================================================================================

void DirectionalLight::OnUpdate()
{
	if (mIsDirty)
	{
		mProperties.mPos = mAbsolutePos;
		mProperties.mDir = mAbsoluteRot.GetForward();
	}
}

//============================================================================================================
// Fill the renderable object and visible light lists
//============================================================================================================

bool DirectionalLight::OnFill (FillParams& params)
{
	if (mProperties.mDiffuse.IsVisibleRGB() || mProperties.mAmbient.IsVisibleRGB())
	{
		params.mDrawQueue.Add(this);
	}
	return true;
}

//============================================================================================================
// Draw the light
//============================================================================================================

void DirectionalLight::OnDrawLight (TemporaryStorage& storage, bool setStates)
{
	// Activate the proper states
	if (setStates)
	{
		if (mShader00 == 0)
		{
			mShader00 = mGraphics->GetShader("[R5] Directional Light");
			mShader01 = mGraphics->GetShader("[R5] Directional Light (AO)");
			mShader10 = mGraphics->GetShader("[R5] Directional Light (Shadow)");
			mShader11 = mGraphics->GetShader("[R5] Directional Light (Shadow + AO)");
		}

		IShader* shader = mShader00;

		if (storage.GetShadow() != 0)
		{
			shader = (storage.GetAO() != 0) ? mShader11 : mShader10;
		}
		else if (storage.GetAO() != 0)
		{
			shader = mShader01;
		}

		// No depth test as directional light has no volume
		mGraphics->SetActiveTexture(0, storage.GetDepth());
		mGraphics->SetActiveTexture(1, storage.GetNormal());
		mGraphics->SetActiveTexture(2, storage.GetShadow());
		mGraphics->SetActiveTexture(3, storage.GetAO());
		mGraphics->SetDepthTest(false);
		mGraphics->SetScreenProjection(true);
		mGraphics->ResetModelViewMatrix();
		mGraphics->SetActiveShader(shader);
	}

	// Draw the light
	mGraphics->SetActiveLight(0, &mProperties);
	mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
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

	if		( tag == "Ambient"		&&  value >> color	) SetAmbient(color);
	else if ( tag == "Diffuse" 		&&  value >> color	) SetDiffuse(color);
	else if ( tag == "Specular"		&&  value >> color	) SetSpecular(color);
	else if ( tag == "Brightness"	&&  value >> f		) SetBrightness(f);
	else if ( tag == "Shadows")			value >> mProperties.mShadows;
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
	node.AddChild("Shadows",	mProperties.mShadows);
}