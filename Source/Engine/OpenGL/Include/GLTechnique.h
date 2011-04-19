#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// OpenGL implementation of the render group
// Author: Michael Lyashenko
//============================================================================================================

class GLTechnique : public ITechnique
{
protected:

	String	mName;
	bool	mFog;
	bool	mDepthWrite;
	bool	mDepthTest;
	bool	mColorWrite;
	bool	mAlphaTest;
	bool	mWireframe;
	byte	mLighting;
	byte	mBlending;
	byte	mCulling;
	byte	mSorting;
	bool	mSerializable;

public:

	GLTechnique (const String& name);
	virtual ~GLTechnique() {}

	virtual const String& GetName()	const	{ return mName;			}
	virtual bool GetFog()			const	{ return mFog;			}
	virtual bool GetDepthWrite()	const	{ return mDepthWrite;	}
	virtual bool GetDepthTest()		const	{ return mDepthTest;	}
	virtual bool GetColorWrite()	const	{ return mColorWrite;	}
	virtual bool GetAlphaTest()		const	{ return mAlphaTest;	}
	virtual bool GetWireframe()		const	{ return mWireframe;	}
	virtual byte GetLighting()		const	{ return mLighting;		}
	virtual byte GetBlending()		const	{ return mBlending;		}
	virtual byte GetCulling()		const	{ return mCulling;		}
	virtual byte GetSorting()		const	{ return mSorting;		}

	virtual void SetFog			(bool val)	{ mFog			= val;  mSerializable = true; }
	virtual void SetDepthWrite	(bool val)	{ mDepthWrite	= val;  mSerializable = true; }
	virtual void SetDepthTest	(bool val)	{ mDepthTest	= val;  mSerializable = true; }
	virtual void SetColorWrite	(bool val)	{ mColorWrite	= val;  mSerializable = true; }
	virtual void SetAlphaTest	(bool val)	{ mAlphaTest	= val;  mSerializable = true; }
	virtual void SetWireframe	(bool val)	{ mWireframe	= val;  mSerializable = true; }
	virtual void SetLighting	(byte val)	{ mLighting		= val;  mSerializable = true; }
	virtual void SetBlending	(byte val)	{ mBlending		= val;  mSerializable = true; }
	virtual void SetCulling		(byte val)	{ mCulling		= val;  mSerializable = true; }
	virtual void SetSorting		(byte val)	{ mSorting		= val;  mSerializable = true; }

	virtual bool IsSerializable() const		{ return mSerializable; }
	virtual void SetSerializable (bool val)	{ mSerializable = val;  }

	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	virtual bool SerializeTo (TreeNode& root) const;
};