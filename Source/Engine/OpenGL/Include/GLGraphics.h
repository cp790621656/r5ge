#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Higher level of renderer API interaction, handles resource management for graphical resources
//============================================================================================================

class GLGraphics : public GLController
{
private:

	struct DelegateEntry
	{
		DelayedDelegate callback;
		void* param;
	};

	typedef Array<DelegateEntry> Delegates;

protected:

	Thread::IDType		mThread;
	IVBO*				mSkyboxVBO;
	IVBO*				mSkyboxIBO;
	uint				mQuery;

	// Resources
	PointerArray<GLVBO>	mVbos;
	PointerArray<GLFBO>	mFbos;
	Delegates			mDelegates;
	Techniques			mTechs;
	Materials			mMaterials;
	Textures			mTextures;
	Shaders				mShaders;
	Fonts				mFonts;

public:

	GLGraphics();
	~GLGraphics();

private:

	// Delegate functions for common shader uniforms
	void SetUniform_EyePos		(const String& name, Uniform& uniform);
	void SetUniform_PixelSize	(const String& name, Uniform& uniform);
	void SetUniform_ClipRange	(const String& name, Uniform& uniform);
	void SetUniform_PM			(const String& name, Uniform& uniform);
	void SetUniform_IVM			(const String& name, Uniform& uniform);
	void SetUniform_IPM			(const String& name, Uniform& uniform);
	void SetUniform_IVRM		(const String& name, Uniform& uniform);
	void SetUniform_WTM			(const String& name, Uniform& uniform);
	void SetUniform_WRM			(const String& name, Uniform& uniform);

public:

	// Returns whether the specified point would be visible if rendered
	virtual bool IsPointVisible (const Vector3f& v);

	// Converts screen coordinates to world coordinates and vice versa
	virtual Vector3f  ConvertTo3D (const Vector2i& v);
	virtual Vector2i  ConvertTo2D (const Vector3f& v);

	// Returns the distance between 'v' and the depth-determined point
	// of intersection on the ray from the camera's eyepoint to 'v'
	//virtual float GetDistanceToDepthAt (const Vector3f& v);

	// Initialize/release the graphics manager
	virtual bool Init();
	virtual void Release();

	// Adds a delayed callback function that should be executed on the next frame (at BeginFrame)
	virtual void ExecuteBeforeNextFrame(DelayedDelegate callback, void* param);

	// Clear the screen or the off-screen target, rendering the skybox if necessary (pre-render)
	virtual void Clear (bool color = true, bool depth = true, bool stencil = true);

	// Pre/post-render
	virtual void BeginFrame();
	virtual void EndFrame();

	// Draws a pre-defined drawable object such as a skybox or a full-screen quad
	virtual uint Draw (uint drawable);

	// Direct access to managed resource arrays
	virtual Techniques&	GetAllTechniques()	{ return mTechs;		}
	virtual Materials&	GetAllMaterials()	{ return mMaterials;	}
	virtual Textures&	GetAllTextures()	{ return mTextures;		}
	virtual Shaders&	GetAllShaders()		{ return mShaders;		}
	virtual Fonts&		GetAllFonts()		{ return mFonts;		}

	// Managed unnamed resources
	virtual IVBO*			CreateVBO();
	virtual IRenderTarget*	CreateRenderTarget();

	// Resource removal
	virtual void DeleteVBO			(const IVBO*			ptr);
	virtual void DeleteRenderTarget	(const IRenderTarget*	ptr);

	// Managed named resources
	virtual ITechnique*		GetTechnique	(const String& name, bool createIfMissing = true);
	virtual IMaterial*		GetMaterial		(const String& name, bool createIfMissing = true);
	virtual ITexture*		GetTexture		(const String& name, bool createIfMissing = true);
	virtual IShader*		GetShader		(const String& name, bool createIfMissing = true);
	virtual IFont*			GetFont			(const String& name, bool createIfMissing = true);

	// Serialization
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	virtual bool SerializeTo (TreeNode& root) const;
};