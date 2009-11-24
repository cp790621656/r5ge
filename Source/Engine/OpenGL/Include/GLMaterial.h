#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// GLMaterial management
//============================================================================================================

class GLMaterial : public IMaterial
{
protected:

	IGraphics*			mGraphics;		// Materials need to know what graphics manager created them
	String				mName;			// Name of the material
	Color				mDiffuse;		// Diffuse color
	Color				mSpecular;		// Specular color
	float				mGlow;			// Percentage of Color unaffected by diminishing light
	float				mAdt;			// Alpha discard threshold for alpha testing
	uint				mMask;			// Mask of available techniques, based on (1 << ITechnique::GetID())
	DrawMethods	mMethods;		// Array of options for various rendering methods
	bool				mSerializable;	// Whether the material is serializable (some may not be)

public:

	GLMaterial(const String& name) : mGraphics(0), mName(name), mSerializable(false) { Release(); }
	~GLMaterial() {}

private:

	// Allow the OpenGL engine class to set the graphics
	friend class GLGraphics;
	void _SetGraphics(IGraphics* graphics)	{ mGraphics = graphics; }

public:

	virtual void Release();

	virtual const String&	GetName()		 const		{ return mName;		}
	virtual const Color&	GetDiffuse()	 const		{ return mDiffuse;	}
	virtual const Color&	GetSpecular()	 const		{ return mSpecular;	}
	virtual float			GetGlow()		 const		{ return mGlow;		}
	virtual float			GetADT()		 const		{ return mAdt;		}

	virtual void SetName		(const String& val)		{ mSerializable = true;  mName		= val; }
	virtual void SetDiffuse		(const Color& val)		{ mSerializable = true;  mDiffuse	= val; }
	virtual void SetSpecular	(const Color& val)		{ mSerializable = true;  mSpecular	= val; }
	virtual void SetGlow		(float val)				{ mSerializable = true;  mGlow		= val; }
	virtual void SetADT			(float val)				{ mSerializable = true;  mAdt		= val; }

	// Material options differ for every technique
	virtual uint				GetTechniqueMask()	 const { return mMask; }
	virtual const DrawMethods&	GetAllDrawMethods()  const { return mMethods; }
	virtual		  DrawMethod*	GetDrawMethod		(const ITechnique* t, bool createIfMissing = true);
	virtual const DrawMethod*	GetVisibleMethod	(const ITechnique* t) const;
	virtual		  DrawMethod*	GetVisibleMethod	(const ITechnique* t) { return (mDiffuse.GetColor4ub().a == 0) ? 0 : GetDrawMethod(t, false); }
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