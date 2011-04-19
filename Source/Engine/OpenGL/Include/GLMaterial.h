#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// OpenGL Material implementation
// Author: Michael Lyashenko
//============================================================================================================

class GLMaterial : public IMaterial
{
protected:

	IGraphics*	mGraphics;			// Materials need to know what graphics manager created them
	String		mName;				// Name of the material
	Color4f		mDiffuse;			// Diffuse color
	float		mGlow;				// Percentage of Color unaffected by diminishing light
	float		mSpecularHue;		// How much the specular component is affected by material's color
	float		mSpecularity;		// Specularity factor
	float		mShininess;			// Shininess of the material
	float		mReflectiveness;	// Material's reflectiveness
	float		mOcclusion;			// How much the material should be affected by ambient occlusion
	float		mAlphaCutoff;		// Alpha cutoff threshold for alpha testing
	uint		mMask;				// Mask of available techniques
	DrawMethods	mMethods;			// Array of options for various rendering methods
	bool		mSerializable;		// Whether the material is serializable (some may not be)

public:

	GLMaterial(const String& name) : mGraphics(0), mName(name), mSerializable(false) { Release(); }
	~GLMaterial() {}

private:

	// Allow the OpenGL engine class to set the graphics
	friend class GLGraphics;
	void _SetGraphics(IGraphics* graphics)	{ mGraphics = graphics; }

public:

	virtual void Release();

	virtual const String&	GetName()			const	{ return mName;				}
	virtual const Color4f&	GetDiffuse()		const	{ return mDiffuse;			}
	virtual float			GetGlow()			const	{ return mGlow;				}
	virtual float			GetSpecularHue()	const	{ return mSpecularHue;		}
	virtual float			GetSpecularity()	const	{ return mSpecularity;		}
	virtual float			GetShininess()		const	{ return mShininess;		}
	virtual float			GetReflectiveness()	const	{ return mReflectiveness;	}
	virtual float			GetOcclusion()		const	{ return mOcclusion;		}
	virtual float			GetAlphaCutoff()	const	{ return mAlphaCutoff;		}

	virtual void SetName			(const String& val)	{ mSerializable = true;  mName			 = val; }
	virtual void SetDiffuse			(const Color4f& val){ mSerializable = true;  mDiffuse		 = val; }
	virtual void SetGlow			(float val)			{ mSerializable = true;  mGlow			 = val; }
	virtual void SetSpecularHue		(float val)			{ mSerializable = true;  mSpecularHue	 = val; }
	virtual void SetSpecularity		(float val)			{ mSerializable = true;  mSpecularity	 = val; }
	virtual void SetShininess		(float val)			{ mSerializable = true;  mShininess		 = val; }
	virtual void SetReflectiveness	(float val)			{ mSerializable = true;  mReflectiveness = val; }
	virtual void SetOcclusion		(float val)			{ mSerializable = true;  mOcclusion		 = val; }
	virtual void SetAlphaCutoff		(float val)			{ mSerializable = true;  mAlphaCutoff	 = val; }

	// Material options differ for every technique
	virtual uint				GetTechniqueMask()	 const { return mMask; }
	virtual const DrawMethods&	GetAllDrawMethods()  const { return mMethods; }
	virtual		  DrawMethod*	GetDrawMethod		(const ITechnique* t, bool createIfMissing = true);
	virtual const DrawMethod*	GetVisibleMethod	(const ITechnique* t) const;
	virtual		  DrawMethod*	GetVisibleMethod	(const ITechnique* t) { return (mDiffuse.a < FLOAT_TOLERANCE) ? 0 : GetDrawMethod(t, false); }
	virtual void				DeleteDrawMethod	(const ITechnique* t);
	virtual void				ClearAllDrawMethods();

	// Whether the material will be serialized out on save
	// (many materials come from model files, so saving isn't necessary)
	virtual bool IsSerializable() const		{ return mSerializable; }
	virtual void SetSerializable(bool val)	{ mSerializable = val;  }

	// Serialization
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	virtual bool SerializeTo (TreeNode& root) const;
};