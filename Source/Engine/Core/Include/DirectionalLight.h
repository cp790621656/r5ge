#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Directional light
//============================================================================================================

class DirectionalLight : public Object
{
public:

	// Struct that stores all the necessary final parameters passed back to the graphics engine
	struct Params : public ILight
	{
		Vector3f	mPos;
		Vector3f	mDir;
		Color4f		mAmbient;
		Color4f		mDiffuse;
		Color4f		mSpecular;

		virtual uint			GetLightType()	 const	{ return ILight::Type::Directional; }
		virtual const Color4f&	GetAmbient()	 const	{ return mAmbient;	}
		virtual const Color4f&	GetDiffuse()	 const	{ return mDiffuse;	}
		virtual const Color4f&	GetSpecular()	 const	{ return mSpecular;	}
		virtual const Vector3f&	GetPosition()	 const	{ return mPos;		}
		virtual const Vector3f&	GetDirection()	 const	{ return mDir;		}
		virtual const Vector3f*	GetAttenParams() const	{ return 0;			}
		virtual const Vector3f*	GetSpotParams()	 const	{ return 0;			}

		bool IsVisible() const { return (mDiffuse.IsVisibleRGB() || mAmbient.IsVisibleRGB()); }
	};

protected:

	Color3f		mAmbient;		// Ambient color
	Color3f		mDiffuse;		// Diffuse color
	Color3f		mSpecular;		// Specular color
	float		mBrightness;	// Light's brighness
	Params		mParams;		// Outgoing parameters

public:

	DirectionalLight();

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Directional Light", DirectionalLight, Object, Object);

protected:

	// Update the 'mParams' light colors
	void _UpdateColors();

	// Update the light parameters
	virtual void OnUpdate();

	// Cull the object based on the viewing frustum
	virtual CullResult OnCull (CullParams& params, bool isParentVisible, bool render);

public:

	const Color3f&	GetAmbient()	const { return mAmbient;	 }
	const Color3f&	GetDiffuse()	const { return mDiffuse;	 }
	const Color3f&	GetSpecular()	const { return mSpecular;	 }
	float			GetBrightness() const { return mBrightness; }

	void SetAmbient		(const Color3f& c)	{ mAmbient	= c; _UpdateColors(); }
	void SetDiffuse		(const Color3f& c)	{ mDiffuse	= c; _UpdateColors(); }
	void SetSpecular	(const Color3f& c)	{ mSpecular = c; _UpdateColors(); }
	void SetBrightness	(float val)			{ mBrightness = val; _UpdateColors(); }

protected:

	// Serialization
	virtual void OnSerializeTo	 ( TreeNode& root ) const;
	virtual bool OnSerializeFrom ( const TreeNode& root );
};