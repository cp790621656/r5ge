#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Object point light
//============================================================================================================

class PointLight : public Object
{
protected:

	Color3f		mAmbient;		// Ambient color
	Color3f		mDiffuse;		// Diffuse color
	Color3f		mSpecular;		// Specular color
	float		mBrightness;	// Light's brighness
	float		mRange;			// Range of the light
	float		mPower;			// Attenuation power
	Light		mLight;			// Outgoing parameters

public:

	PointLight();

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Point Light", PointLight, Object, Object);

private:

	// Updates appropriate fields in 'mParams'
	void _UpdateColors();
	void _UpdateAtten();

public:

	const Color3f&	GetAmbient()	const { return mAmbient;	}
	const Color3f&	GetDiffuse()	const { return mDiffuse;	}
	const Color3f&	GetSpecular()	const { return mSpecular;	}
	float			GetBrightness() const { return mBrightness;	}
	float			GetRange()		const { return mRange;		}
	float			GetPower()		const { return mPower;		}

	void SetAmbient		(const Color3f& c)	{ mAmbient	= c; _UpdateColors(); }
	void SetDiffuse		(const Color3f& c)	{ mDiffuse	= c; _UpdateColors(); }
	void SetSpecular	(const Color3f& c)	{ mSpecular	= c; _UpdateColors(); }
	void SetBrightness	(float val);
	void SetRange		(float val);
	void SetPower		(float val);

protected:

	// Update the light parameters
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

protected:

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& root);
};