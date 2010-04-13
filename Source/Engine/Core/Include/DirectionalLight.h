#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
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

private:

	// Update the 'mParams' light colors
	void _UpdateColors();

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Directional Light", DirectionalLight, Object, Object);

	// Callback that draws point lights
	static void _Draw (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap);

	const Color3f&	GetAmbient()	const { return mAmbient;	}
	const Color3f&	GetDiffuse()	const { return mDiffuse;	}
	const Color3f&	GetSpecular()	const { return mSpecular;	}
	float			GetBrightness() const { return mBrightness;	}

	void SetAmbient		(const Color3f& c)	{ mAmbient		= c;	_UpdateColors(); }
	void SetDiffuse		(const Color3f& c)	{ mDiffuse		= c;	_UpdateColors(); }
	void SetSpecular	(const Color3f& c)	{ mSpecular		= c;	_UpdateColors(); }
	void SetBrightness	(float val)			{ mBrightness	= val;	_UpdateColors(); }

protected:

	// Update the light parameters
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

protected:

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& node) const;
	virtual bool OnSerializeFrom  (const TreeNode& node);
};