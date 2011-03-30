#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Directional light
//============================================================================================================

class DirectionalLight : public LightSource
{
protected:

	Color3f		mAmbient;		// Ambient color
	Color3f		mDiffuse;		// Diffuse color
	Color3f		mSpecular;		// Specular color
	float		mBrightness;	// Light's brightness

	IShader*	mShader00;		// No shadows or AO
	IShader*	mShader01;		// No shadows but with AO
	IShader*	mShader10;		// With shadows and no AO
	IShader*	mShader11;		// With both shadows and AO

	DirectionalLight();

private:

	// Update the 'mParams' light colors
	void _UpdateColors();

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Directional Light", DirectionalLight, Object, Object);

	const Color3f&	GetAmbient()	const { return mAmbient;	}
	const Color3f&	GetDiffuse()	const { return mDiffuse;	}
	const Color3f&	GetSpecular()	const { return mSpecular;	}
	float			GetBrightness() const { return mBrightness;	}

	void SetAmbient		(const Color3f& c)	{ mAmbient		= c;	_UpdateColors(); }
	void SetDiffuse		(const Color3f& c)	{ mDiffuse		= c;	_UpdateColors(); }
	void SetSpecular	(const Color3f& c)	{ mSpecular		= c;	_UpdateColors(); }
	void SetBrightness	(float val)			{ mBrightness	= val;	_UpdateColors(); }

	// Whether the light will be casting shadows
	bool GetCastShadows() const { return mProperties.mShadows;	}
	void SetCastShadows(bool val) { mProperties.mShadows = val; }

protected:

	// Update the light parameters
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draw the light using deferred rendering
	virtual void OnDrawLight (TemporaryStorage& storage, bool setStates);

protected:

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& node) const;
	virtual bool OnSerializeFrom  (const TreeNode& node);
};