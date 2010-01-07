#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Directional light
//============================================================================================================

class DirectionalLight : public Object
{
protected:

	Color3f		mAmbient;		// Ambient color
	Color3f		mDiffuse;		// Diffuse color
	Color3f		mSpecular;		// Specular color
	float		mBrightness;	// Light's brightness
	Light		mLight;			// Light's outgoing parameters

	DirectionalLight();

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

protected:

	// Update the 'mParams' light colors
	void _UpdateColors();

	// Update the light parameters
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

protected:

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& root) const;
	virtual bool OnSerializeFrom  (const TreeNode& root);
};