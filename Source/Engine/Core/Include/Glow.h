#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Screen-aligned billboard
//============================================================================================================

class Glow : public Object
{
public:

	struct Type
	{
		enum
		{
			Point = 0,
			Directional
		};
	};

protected:

	uint			mType;			// Point or directional
	Color4f			mColor;			// It would be a pretty sad glow if it couldn't be colored
	float			mMag;			// Magnification factor the foreground texture's radius is multiplied by
	const ITexture*	mBackground;	// Background texture, rendered in transparent object mode
	const ITexture*	mForeground;	// Foreground texture, rendered in overlay mode after everything else

private:

	Vector3f		mIntensity;		// Intensity factors for smooth interpolation (start, current, target)
	float			mTimeStamp;		// Timestamp used for gradual alpha changes
	Matrix43		mMat;			// Transformation matrix, recalculated once per frame

public:

	Glow();

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Glow", Glow, Object, Object);

public:

	uint			GetGlowType()			const	{ return mType;			}
	const Color4f&  GetColor()				const	{ return mColor;		}
	float			GetMagnification()		const	{ return mMag;			}
	const ITexture* GetBackgroundTexture()	const	{ return mBackground;	}
	const ITexture* GetForegroundTexture()	const	{ return mForeground;	}

	void SetGlowType		 (uint type)			{ mType			= type;	}
	void SetColor			 (const Color4f& color) { mColor		= color;}
	void SetMagnification	 (float val)			{ mMag			= val;	}
	void SetBackgroundTexture(const ITexture* tex)	{ mBackground	= tex;	}
	void SetForegroundTexture(const ITexture* tex)	{ mForeground	= tex;	}

protected:

	// Changes the alpha value that the light animates to
	void _SetTargetAlpha (float target);

	// Updates the light's alpha
	virtual void OnUpdate();

	// Culls objects based on the viewing frustum, and in this case -- activates lights
	virtual bool OnCull (CullParams& params, bool isParentVisible, bool render);

	// Draws the light on-screen if it's visible
	virtual uint OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut);

protected:

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& root);
};