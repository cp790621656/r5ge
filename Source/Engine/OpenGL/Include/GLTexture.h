#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Extended DevIL texture loading/management class to support OpenGL related stuff
//============================================================================================================

class GLTexture : public ITexture
{
protected:

	String		mName;				// Every texture needs a name
	String		mFilename;			// When the texture gets the Save request, this value is updated
	GLGraphics*	mGraphics;			// GLGraphics manager that has created this texture
	GLTexture*	mReplacement;		// Replacement texture that overrides this one
	Image		mTex[6];			// Up to 6 raw textures
	bool		mCheckForSource;	// Whether the texture should be checked to see if it has a valid source
	uint		mType;				// Texture type (ITexture::TwoDimensional, etc)
	uint		mGlID;				// Associated OpenGL texture ID
	uint		mGlType;			// Saved OpenGL Texture type (GL_TEXTURE_2D, etc)
	uint		mFormat;			// Texture format, such as ITexture::Format::RGB
	uint		mRequestedFormat;	// Original requested format (mFormat can change, if not supported)
	Vector2i	mSize;				// Size of the texture
	uint		mDepth;				// Depth of the texture
	ulong		mSizeInMemory;		// Size in video memory that the texture is currently taking up
	ulong		mTimestamp;			// Timestamp of the last time Activate() was called

	uint		mWrapMode;			// Texture wrapping setting
	uint		mCompareMode;		// Texture compare mode
	uint		mFilter;			// Texture filtering setting
	uint		mActiveWrapMode;	// Active wrapping setting
	uint		mActiveFilter;		// Active filtering
	uint		mActiveCompareMode;	// Active color compare mode

	int			mInFormat;			// Cached internal GL format
	uint		mDataType;			// Cached internal data type
	bool		mMipmapsGenerated;	// Whether mip-maps were generated
	bool		mRegenMipmap;		// Whether to re-generate the mip-map
	uint		mActiveAF;			// Active anisotropic filter setting
	bool		mSerializable;		// Whether the texture should be serialized on save (if it comes from a model file, it won't be)

	Thread::Lockable mLock;

private:

	// Only the GLGraphics manager should be able to create new textures
	friend class GLGraphics;
	GLTexture(const String& name, IGraphics* graphics);

public:

	virtual ~GLTexture() { _InternalRelease(false); }
	virtual void Release();

private:

	// Tries to load the texture by its name
	void _CheckForSource();

	// Internal function: Releases the associated OpenGL texture
	void _InternalRelease (bool delayExecution);

	// Internal function: Creates the texture
	void _Create();

	// Uploads the texture to the videocard, or simply returns a texture ID
	uint _GetOrCreate();

	// OpenGL min/max filters
	uint _GetGLMinFilter() const;
	uint _GetGLMagFilter() const;

public:

	// Some functions should be overwritten
	virtual const String&	GetName()	const	{ return mName; }
	virtual const Vector2i& GetSize()	const	{ return (mReplacement == 0) ? mSize : mReplacement->GetSize(); }

	virtual bool	IsValid()			const;
	virtual uint	GetDepth()			const	{ return (mReplacement == 0) ? mDepth  : mReplacement->GetDepth(); }
	virtual uint	GetFormat()			const	{ return (mReplacement == 0) ? mFormat : mReplacement->GetFormat(); }
	virtual uint	GetFiltering()		const	{ return (mReplacement == 0) ? mFilter : mReplacement->GetFiltering(); }
	virtual uint	GetWrapMode()		const	{ return (mReplacement == 0) ? mWrapMode : mReplacement->GetWrapMode(); }
	virtual uint	GetCompareMode()	const	{ return (mReplacement == 0) ? mCompareMode : mReplacement->GetCompareMode(); }
	virtual ulong	GetSizeInMemory()	const	{ return (mReplacement == 0) ? mSizeInMemory : mReplacement->GetSizeInMemory(); }
	virtual uint	GetMaxSize()		const;
	virtual uint	GetType()			const	{ return (mReplacement == 0) ? mType : mReplacement->GetType(); }
	virtual ulong	GetLastUsedTime()	const	{ return (mReplacement == 0) ? mTimestamp : mReplacement->GetLastUsedTime(); }

	// Returns the valid path to the texture's source
	virtual const String& GetSource (uint index) const;

	// Saves the image's color data into the specified memory buffer
	// NOTE: Must be called from the graphics thread!
	virtual bool GetBuffer (Memory& mem);

	// Wrapping mode and filtering
	virtual void SetWrapMode (uint wrapMode);
	virtual void SetFiltering (uint filtering);
	virtual void SetCompareMode (uint compareMode);
	virtual void InvalidateMipmap() { if (mReplacement == 0) mReplacement->InvalidateMipmap(); else mRegenMipmap = true; }

	// Activates the texture and returns its identifier
	virtual uint Activate();

	// Reserve an internal texture of specified dimensions
	virtual bool Reserve (uint width, uint height, uint depth = 1, uint format = ITexture::Format::RGBA, uint msaa = 0);

	// Load a single texture from the specified file
	virtual bool Load( const String& file, uint textureFormat = Format::Optimal );

	// Load a cube map using the six specified textures
	virtual bool Load(	const String&	up,
						const String&	down,
						const String&	north,
						const String&	east,
						const String&	south,
						const String&	west,
						uint			format = Format::Optimal );

	// Assign texture data manually
	virtual bool Set(	const void*		buffer,
						uint			width,
						uint			height,
						uint			depth,
						uint			dataFormat,
						uint			format = Format::Optimal );

	// Assign the cube map data manually
	virtual bool Set(	const void*	up,
						const void*	down,
						const void*	north,
						const void*	east,
						const void*	south,
						const void*	west,
						uint width, uint height,
						uint dataFormat,
						uint textureFormat = Format::Optimal );

	// Replacement texture that this texture is pointing to
	virtual const ITexture* GetReplacement() const { return mReplacement; }

	// Replacement textures can be used to temporarily (or permanently) replace one texture with another.
	// This feature is most useful for frequently-changing textures, such as shadowmaps, as they tend to differ
	// with each render target, but still need to be referenced by material techniques.
	virtual void SetReplacement (ITexture* tex) { if (tex == this) tex = 0; mReplacement = (GLTexture*)tex; }

	// Adjusts the anisotropic filtering level for all textures
	static void SetDefaultAF (uint level);

public:

	// Serialization
	virtual bool IsSerializable() const { return (mSerializable && mTex[0].GetSource().IsValid()); }
	virtual void SetSerializable(bool val) { mSerializable = val; }
	virtual bool SerializeTo (TreeNode& root) const;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
};