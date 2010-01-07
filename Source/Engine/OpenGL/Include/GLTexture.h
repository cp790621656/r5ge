#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Extended DevIL texture loading/management class to support OpenGL related stuff
//============================================================================================================

class GLTexture : public ITexture
{
protected:

	String		mName;				// Every texture needs a name
	IGraphics*	mGraphics;			// GLGraphics manager that has created this texture
	Image		mTex[6];			// Up to 6 raw textures
	bool		mCheckForSource;	// Whether the texture should be checked to see if it has a valid source
	uint		mGlID;				// Associated OpenGL texture ID
	uint		mType;				// Texture type (ITexture::TwoDimensional, etc)
	uint		mGlType;			// Saved OpenGL Texture type (GL_TEXTURE_2D, etc)
	uint		mFormat;			// Texture format, such as ITexture::Format::RGB
	uint		mRequestedFormat;	// Original requested format (mFormat can change, if not supported)
	Vector2i	mSize;				// Size of the texture
	uint		mDepth;				// Depth of the texture
	ulong		mSizeInMemory;		// Size in video memory that the texture is currently taking up
	ulong		mTimestamp;			// Timestamp of the last time GetTextureID() was called
	uint		mWrapMode;			// Texture wrapping setting
	uint		mFilter;			// Texture filtering setting
	uint		mActiveWrapMode;	// Active wrapping setting
	uint		mActiveFilter;		// Active filtering
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
	void _InternalRelease(bool delayExecution);

	// Uploads the texture to the videocard, or simply returns a texture ID
	uint _GetOrCreate();

	// Internal function: Creates the texture
	void _Create();

	// Binds the specified texture
	bool _BindTexture (uint glType, uint glID);

public:

	// Some functions should be overwritten
	virtual const String&	GetName()	const	{ return mName;			}
	virtual const Vector2i& GetSize()	const	{ return mSize;			}

	virtual bool	IsValid()			const;
	virtual uint	GetDepth()			const	{ return mDepth;		}
	virtual uint	GetFormat()			const	{ return mFormat;		}
	virtual uint	GetFiltering()		const	{ return mFilter;		}
	virtual uint	GetWrapMode()		const	{ return mWrapMode;		}
	virtual ulong	GetSizeInMemory()	const	{ return mSizeInMemory;	}
	virtual uint	GetMaxSize()		const;
	virtual uint	GetTextureID()		const	{ return (const_cast<GLTexture*>(this))->_GetOrCreate(); }
	virtual uint	GetType()			const	{ return mType;			}
	virtual ulong	GetLastUsedTime()	const	{ return mTimestamp;	}

	// Returns the valid path to the texture's source
	virtual const String&	GetSource (uint index) const { return (index < 6 ? mTex[index].GetSource() : mTex[0].GetSource()); }

	// Wrapping mode and filtering
	virtual void SetWrapMode  (uint wrapMode);
	virtual void SetFiltering (uint filtering);
	virtual void InvalidateMipmap() { mRegenMipmap = true; }

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

	// Adjusts the anisotropic filtering level for all textures
	static void SetDefaultAF (uint level);

public:

	// Serialization
	virtual bool IsSerializable() const { return (mSerializable && mTex[0].GetSource().IsValid()); }
	virtual void SetSerializable(bool val) { mSerializable = val; }
	virtual bool SerializeTo (TreeNode& root) const;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
};