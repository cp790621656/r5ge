#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

const uint convertTextureTypeToGL [5] = { 0, GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

//============================================================================================================
// Current active anisotropy level
//============================================================================================================

uint mDefaultAF = 0;

//============================================================================================================
// Deletes an OpenGL texture
//============================================================================================================

inline void _DeleteTexture (uint& texid)
{
	if (texid != 0)
	{
		glDeleteTextures(1, &texid);
		texid = 0;
	}
}

//============================================================================================================
// Delayed callback executed by the GLGraphics manager (textures should only be released on the graphics thread)
//============================================================================================================

void DeleteTexture(void* ptr)
{
	uint texid = (uint)(ulong)ptr;
	_DeleteTexture(texid);
}

//============================================================================================================
// Not all formats may be supported -- this returns the format that is supported by the videocard
//============================================================================================================

inline uint _GetSupportedFormat (uint format)
{
	switch (format)
	{
	case ITexture::Format::DXT1:		return g_caps.mDXTCompression ? ITexture::Format::DXT1 : ITexture::Format::RGB;
	case ITexture::Format::DXT3:		return g_caps.mDXTCompression ? ITexture::Format::DXT3 : ITexture::Format::RGB;
	case ITexture::Format::DXTN:		return g_caps.mDXTCompression ? ITexture::Format::DXT5 : ITexture::Format::RGBA;
	case ITexture::Format::DXT5:		return g_caps.mDXTCompression ? ITexture::Format::DXT5 : ITexture::Format::RGBA;
	case ITexture::Format::Float:		return g_caps.mFloat32Format ? ITexture::Format::Float : ITexture::Format::Alpha;
	case ITexture::Format::RGB16F:		return g_caps.mFloat16Format ? ITexture::Format::RGB16F  : (g_caps.mFloat32Format ? ITexture::Format::RGB32F  : ITexture::Format::RGB);
	case ITexture::Format::RGBA16F:		return g_caps.mFloat16Format ? ITexture::Format::RGBA16F : (g_caps.mFloat32Format ? ITexture::Format::RGBA32F : ITexture::Format::RGBA);
	case ITexture::Format::RGB32F:		return g_caps.mFloat32Format ? ITexture::Format::RGB32F  : (g_caps.mFloat16Format ? ITexture::Format::RGB16F  : ITexture::Format::RGB);
	case ITexture::Format::RGBA32F:		return g_caps.mFloat32Format ? ITexture::Format::RGBA32F : (g_caps.mFloat16Format ? ITexture::Format::RGBA16F : ITexture::Format::RGBA);
	}
	return format;
}

//============================================================================================================
// helper function that figures out the appropriate matching OpenGL format
//============================================================================================================

inline int _GetGLFormat (uint format)
{
	switch (format)
	{
	case ITexture::Format::Alpha:			return GL_INTENSITY;
	case ITexture::Format::Luminance:		return GL_LUMINANCE_ALPHA;
	case ITexture::Format::RGB:				return GL_RGB;
	case ITexture::Format::RGBA:			return GL_RGBA;
	case ITexture::Format::RGB30A2:			return GL_RGB10_A2;
	case ITexture::Format::DXT1:			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case ITexture::Format::DXT3:			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case ITexture::Format::DXTN:			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case ITexture::Format::DXT5:			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case ITexture::Format::Float:			return GL_INTENSITY32F_ARB;
	case ITexture::Format::RGB16F:			return GL_RGB16F_ARB;
	case ITexture::Format::RGBA16F:			return GL_RGBA16F_ARB;
	case ITexture::Format::RGB32F:			return GL_RGB32F_ARB;
	case ITexture::Format::RGBA32F:			return GL_RGBA32F_ARB;
	case ITexture::Format::Depth:			return GL_DEPTH_COMPONENT24;
	case ITexture::Format::DepthStencil:	return GL_DEPTH24_STENCIL8;
	}
	return 0;
}

//============================================================================================================
// Returns the maximum supported texture width/height
//============================================================================================================

uint GLTexture::GetMaxSize() const
{
	return g_caps.mMaxTextureSize;
}

//============================================================================================================
// Adjusts the anisotropic filtering level for all textures
//============================================================================================================

void GLTexture::SetDefaultAF (uint level)
{
	mDefaultAF = (level < g_caps.mMaxAnisotropicLevel) ? level : g_caps.mMaxAnisotropicLevel;
}

//============================================================================================================
// All textures need to be created with a name
//============================================================================================================

GLTexture::GLTexture (const String& name, IGraphics* graphics) :
	mName				(name),
	mGraphics			(graphics),
	mCheckForSource		(true),
	mGlID				(0),
	mType				(Type::Invalid),
	mGlType				(0),
	mFormat				(Format::Invalid),
	mRequestedFormat	(Format::Invalid),
	mDepth				(0),
	mSizeInMemory		(0),
	mTimestamp			(Time::GetMilliseconds()),
	mWrapMode			(WrapMode::Default),
	mFilter				(Filter::Default),
	mActiveWrapMode		(WrapMode::Default),
	mActiveFilter		(Filter::Default),
	mMipmapsGenerated	(false),
	mRegenMipmap		(false),
	mActiveAF			(0),
	mSerializable		(true)
{
}

//============================================================================================================
// Check to see if the texture has valid source
//============================================================================================================

void GLTexture::_CheckForSource()
{
	mCheckForSource = false;

	// Immediately try to load this texture if it's specified as a path name
	if ( mTex[0].Load(mName) )
	{
		mSerializable = false;

		mSize.x = mTex[0].GetWidth();
		mSize.y = mTex[0].GetHeight();
		mDepth  = mTex[0].GetDepth();
		mFormat = mTex[0].GetFormat();
		mType	 = (mDepth > 1) ? Type::ThreeDimensional : Type::TwoDimensional;

		const String& source = mTex[0].GetSource();

		// Default jpegs to DXT3 compression. All other formats use their native formats.
		if ( source.Find(".jpg", false) != source.GetLength() )
		{
			mRequestedFormat = Format::DXT3;
			mFormat = _GetSupportedFormat(mRequestedFormat);
		}
	}
}

//============================================================================================================
// Internal-only function -- releases the associated OpenGL texture
//============================================================================================================

void GLTexture::_InternalRelease(bool delayExecution)
{
	g_caps.DecreaseTextureMemory(mSizeInMemory);

#ifdef _DEBUG
	if (mGlID != 0)
	{
		System::Log("[TEXTURE] Releasing  '%s'", mName.GetBuffer());
		System::Log("          - Size:     %s bytes", String::GetFormattedSize(mSizeInMemory).GetBuffer());
	}
#endif

	if (mGlID != 0)
	{
		if (delayExecution)
		{
			// OpenGL textures should only be destroyed on the graphics thread
			mGraphics->ExecuteBeforeNextFrame(DeleteTexture, (void*)mGlID);
		}
		else
		{
			// Optimized version for when we are inside the graphics thread
			glDeleteTextures(1, &mGlID);
		}
		mGlID = 0;
	}

	for (uint i = 0; i < 6; ++i)
		mTex[i].Release();

	mCheckForSource		= true;
	mType				= Type::Invalid;
	mGlType				= 0;
	mFormat				= Format::Invalid;
	mRequestedFormat	= Format::Invalid;
	mSize.x				= 0;
	mSize.y				= 0;
	mDepth				= 0;
	mSizeInMemory		= 0;
	mWrapMode			= WrapMode::Default;
	mFilter				= Filter::Default;
	mActiveWrapMode		= WrapMode::Default;
	mActiveFilter		= Filter::Default;
	mMipmapsGenerated	= false;
	mActiveAF			= 0;
	mSerializable		= true;
}

//============================================================================================================
// Since this function can be called from anywhere, we want a part of it to be delayed
//============================================================================================================

void GLTexture::Release()
{
	mLock.Lock();
	_InternalRelease(true);
	mLock.Unlock();
}

//============================================================================================================
// Retrieves the associated OpenGL texture ID
//============================================================================================================

uint GLTexture::_GetOrCreate()
{
	bool active (false);

	// If OpenGL texture hasn't been created yet, and there's data present, let's create it
	if ( mGlID == 0 && IsValid() )
	{
		_Create();
		active = true;
	}

	if ( mGlID != 0 )
	{
		// If the filtering has not been specified, choose whatever seems appropriate
		if (mFilter == Filter::Default)
		{
			bool isPowerOfTwo = IsPowerOfTwo(mSize.x) && IsPowerOfTwo(mSize.y);

			if (!isPowerOfTwo)
			{
				// Non-power-of-two textures are not filtered at all by default
				mFilter = Filter::Nearest;
			}
			else if (mFormat & Format::HDR)
			{
				// HDR textures are only linear-filtered
				mFilter = Filter::Linear;
			}
			else
			{
				// All other cases use anisotropic filtering
				mFilter = Filter::Anisotropic;
			}
		}

		// Adjust the filtering and generate mipmaps if necessary
		if (mActiveFilter != mFilter)
		{
			mActiveFilter = mFilter;

			if (!active)
			{
				active = true;
				_BindTexture( mGlType, mGlID );
			}

			if (mFilter & Filter::Mipmap)
			{
				glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (mFilter == Filter::Linear)
			{
				glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
			CHECK_GL_ERROR;
		}

		// Anisotropy is only enabled for mip-mapped textures
		uint af = (mFilter & ITexture::Filter::Mipmap) ? mDefaultAF : 1;

		// Adjust the active texture anisotropy level
		if (mActiveAF != af && af > 0)
		{
			mActiveAF = af;

			if (!active)
			{
				active = true;
				_BindTexture( mGlType, mGlID );
			}

			glTexParameteri( mGlType, GL_TEXTURE_MAX_ANISOTROPY_EXT, mActiveAF );
			CHECK_GL_ERROR;
		}

		// If texture wrapping has not been specified, choose whatever seems appropriate
		if (mWrapMode == WrapMode::Default)
		{
			bool isPowerOfTwo = IsPowerOfTwo(mSize.x) && IsPowerOfTwo(mSize.y);

			if (mFormat & Format::Depth)
			{
				mWrapMode = WrapMode::ClampToOne;
			}
			else if (!isPowerOfTwo || mType == Type::EnvironmentCubeMap)
			{
				mWrapMode = WrapMode::ClampToEdge;
			}
			else
			{
				mWrapMode = WrapMode::Repeat;
			}
		}

		// Adjust the texture wrapping setting if necessary
		if (mActiveWrapMode != mWrapMode)
		{
			mActiveWrapMode = mWrapMode;

			if (!active)
			{
				active = true;
				_BindTexture( mGlType, mGlID );
			}

			if (mWrapMode == WrapMode::Mirror)
			{
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
			}
			else if (mWrapMode == WrapMode::ClampToEdge)
			{
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}
			else if (mWrapMode == WrapMode::ClampToZero)
			{
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

				const float color[] = {0.0f, 0.0f, 0.0f, 0.0f};
				glTexParameterfv(mGlType, GL_TEXTURE_BORDER_COLOR, color);
			}
			else if (mWrapMode == WrapMode::ClampToOne)
			{
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

				const float color[] = {1.0f, 1.0f, 1.0f, 1.0f};
				glTexParameterfv(mGlType, GL_TEXTURE_BORDER_COLOR, color);
			}
			else
			{
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(mGlType, GL_TEXTURE_WRAP_R, GL_REPEAT);
			}
			CHECK_GL_ERROR;
		}

		// Check to see if we need to generate mipmaps
		if ( mFilter & Filter::Mipmap )
		{
			if (!mMipmapsGenerated)
			{
				if (!active)
				{
					active = true;
					_BindTexture( mGlType, mGlID );
				}

				// Generate the mipmap
				glGenerateMipmap(mGlType);

				// Keep track of used memory
				g_caps.DecreaseTextureMemory(mSizeInMemory);

				ulong size	 = 0;
				ulong bpp	 = ITexture::GetBitsPerPixel(mFormat);
				ulong width  = mSize.x;
				ulong height = mSize.y;
				ulong depth  = mDepth;

				// Calculate how much memory the texture is taking with its mip-maps
				while ( width > 1 || height > 1 || depth > 1 )
				{
					ulong current = width * height * depth * bpp;
					current /= 8;
					if (current == 0) current = 1;

					size += current;

					width  = width  >> 1;
					height = height >> 1;
					depth  = depth  >> 1;

					if (width  == 0) width  = 1;
					if (height == 0) height = 1;
					if (depth  == 0) depth  = 1;
				}

				if (mType == ITexture::Type::EnvironmentCubeMap)
					size *= 6;

				// Keep track of video memory used by textures
				g_caps.IncreaseTextureMemory(size);
#ifdef _DEBUG
				System::Log("[TEXTURE] Generated mipmaps for '%s'", mName.GetBuffer());
				System::Log("          - Released  %s bytes", String::GetFormattedSize(mSizeInMemory).GetBuffer());
				System::Log("          - Reserved  %s bytes", String::GetFormattedSize(size).GetBuffer());
#endif
				mSizeInMemory = size;
			}
			else if (mRegenMipmap)
			{
				if (!active)
				{
					active = true;
					_BindTexture( mGlType, mGlID );
				}
				glGenerateMipmap(mGlType);
			}

			mMipmapsGenerated = true;
			mRegenMipmap = false;
			CHECK_GL_ERROR;
		}
	}

	mTimestamp = Time::GetMilliseconds();
	return mGlID;
}

//============================================================================================================
// Internal function: Creates the OpenGL texture
//============================================================================================================
// This function is called on GetOrCreate(), and that one is called before glBindTexture()
// is called. Since OpenGL calls should only be used in one thread, there is no need to
// create resource locking / unlocking for OpenGL texture binding calls.
//============================================================================================================

void GLTexture::_Create()
{
	ASSERT(mType != ITexture::Type::Invalid, "Invalid texture type");

	mLock.Lock();

	// Get one texture handle from OpenGL
	glGenTextures(1, &mGlID);
	ASSERT( mGlID != 0, "glGenTextures() failed!");

	// Figure out the texture's OpenGL type
#ifdef _DEBUG
	mGlType = (mType < 5) ? convertTextureTypeToGL[mType] : 0;
#else
	mGlType = convertTextureTypeToGL[mType];
#endif

	// Bind the texture
	_BindTexture(mGlType, mGlID);

	// Figure out the appropriate texture format
	int inFormat;
	uint dataFormat = mTex[0].GetFormat(), dataType(GL_UNSIGNED_BYTE);

	if ( mTex[0].GetBuffer() == 0 )
	{
		if (mFormat == Format::Depth)
		{
			inFormat = GL_DEPTH_COMPONENT;
		}
		else if (mFormat == Format::DepthStencil)
		{
			inFormat = GL_DEPTH_STENCIL;
			dataType = GL_UNSIGNED_INT_24_8;
		}
		else inFormat = GL_RGB;
	}
	else
	{
		switch ( dataFormat )
		{
			case Format::Alpha:			inFormat = GL_LUMINANCE;		dataType = GL_UNSIGNED_BYTE;	break;
			case Format::Luminance:		inFormat = GL_LUMINANCE_ALPHA;	dataType = GL_UNSIGNED_BYTE;	break;
			case Format::RGB:			inFormat = GL_RGB;				dataType = GL_UNSIGNED_BYTE;	break;
			case Format::RGBA:			inFormat = GL_RGBA;				dataType = GL_UNSIGNED_BYTE;	break;
			case Format::Float:			inFormat = GL_LUMINANCE;		dataType = GL_FLOAT;			break;
			case Format::RGB32F:		inFormat = GL_RGB;				dataType = GL_FLOAT;			break;
			case Format::RGBA32F:		inFormat = GL_RGBA;				dataType = GL_FLOAT;			break;
			default:
				ASSERT(false, "Invalid texture format");
				break;
		};
	}

	// Figure out the appropriate bitrate and OpenGL format
	uint bpp = ITexture::GetBitsPerPixel(mFormat);
	int outFormat = _GetGLFormat(mFormat);

	if ( mGlType == GL_TEXTURE_2D )
	{
		if (mFormat == Format::DXTN)
		{
			// DXT5 format meant for normal maps -- blue color must be stored in the alpha channel
			uint size = mTex[0].GetSize();
			byte* buffer = (byte*)(mTex[0].GetBuffer());

			if (dataFormat == Format::RGB)
			{
				inFormat = GL_RGBA;

				// Since we need the alpha channel, we have to create a new buffer altogether
				uint newSize = size / 3 * 4;
				byte* data = new byte[newSize];
				{
					// Go through
					for (uint o = 0, n = 0; o < size; o += 3, n += 4)
					{
						data[n]   = buffer[o];
						data[n+1] = buffer[o+1];
						data[n+2] = 0;
						data[n+3] = buffer[o+2];
					}

					// Send this new buffer to the videocard
					glTexImage2D(mGlType, 0, outFormat, mSize.x, mSize.y, 0, inFormat, dataType, data);
				}
				delete [] data;
			}
			else if (dataFormat == Format::RGBA)
			{
				// Swap B and A components
				for (uint i = 0; i < size; i+=4)
					Swap(buffer[i+2], buffer[i+3]);

				glTexImage2D(mGlType, 0, outFormat, mSize.x, mSize.y, 0, inFormat, dataType, buffer);
			}
			else
			{
				ASSERT(false, "Unsupported data format for DXTN compression. You must use RGB or RGBA for input data.");
				glTexImage2D(mGlType, 0, outFormat, mSize.x, mSize.y, 0, inFormat, dataType, buffer);
			}
		}
		else
		{
			// Basic 2D texture has a single image source
			glTexImage2D(mGlType, 0, outFormat, mSize.x, mSize.y, 0, inFormat, dataType, mTex[0].GetBuffer());
		}
	}
	else if ( mGlType == GL_TEXTURE_CUBE_MAP )
	{
		// Cubemap texture has 6 source images
		for ( uint i = 0; i < 6; ++i )
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, outFormat,
				mSize.x, mSize.y, 0, inFormat, dataType, mTex[i].GetBuffer());
		}
	}
	else if ( mGlType == GL_TEXTURE_3D )
	{
		// 3D texture
		glTexImage3D(mGlType, 0, outFormat, mSize.x, mSize.y, mDepth, 0, inFormat, dataType, mTex[0].GetBuffer());
	}
	else
	{
		ASSERT(false, "1D textures are not implemented -- use 2D textures instead");
		_InternalRelease(false);
		mLock.Unlock();
		return;
	}

	const char* err = glGetErrorString();

	if (err)
	{
		// Some error occured -- invalidate the texture
		String errText;
		errText.Set("glTexImage returned: '%s'", err);
		WARNING( errText.GetBuffer() );
		_InternalRelease(false);
		mLock.Unlock();
		return;
	}
	else if (mFormat & ITexture::Format::Depth || mGlType == GL_TEXTURE_3D )
	{
		// Depth and 3D textures might not have filtering support, so start with no filtering
		glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		// All other textures start with linear filtering
		glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	mSizeInMemory = 0;

	if		(mGlType == GL_TEXTURE_2D)			mSizeInMemory = (ulong)mSize.x * mSize.y * bpp;
	else if (mGlType == GL_TEXTURE_CUBE_MAP)	mSizeInMemory = (ulong)mSize.x * mSize.y * bpp * 6;
	else if (mGlType == GL_TEXTURE_3D)			mSizeInMemory = (ulong)mSize.x * mSize.y * bpp * mSize.x * 6;

	mSizeInMemory /= 8;

	// Keep track of video memory used by textures
	g_caps.IncreaseTextureMemory(mSizeInMemory);

#ifdef _DEBUG
												System::Log("[TEXTURE] Created '%s'", mName.GetBuffer() );
	if		(mGlType == GL_TEXTURE_2D)			System::Log( "          - Dims:     %u x %u",		mSize.x, mSize.y );
	else if (mGlType == GL_TEXTURE_CUBE_MAP)	System::Log( "          - Dims:     %u x %u x 6",	mSize.x, mSize.y );
	else if (mGlType == GL_TEXTURE_3D)			System::Log( "          - Dims:     %u x %u x %u", mSize.x, mSize.y, mDepth );

	System::Log("          - Format:   %s", ITexture::FormatToString(mFormat));
	System::Log("          - Size:     %s bytes", String::GetFormattedSize(mSizeInMemory).GetBuffer());
#endif

	// Release the memory
	for (uint i = 0; i < 6; ++i)
		mTex[i].Release();

	mLock.Unlock();
	CHECK_GL_ERROR;
}

//============================================================================================================
// Binds the specified texture
//============================================================================================================

inline bool GLTexture::_BindTexture (uint glType, uint glID)
{
	return ((GLGraphics*)mGraphics)->_BindTexture(glType, glID);
}

//============================================================================================================
// Whether the texture can be used
//============================================================================================================

bool GLTexture::IsValid() const
{
	if (mCheckForSource) (const_cast<GLTexture*>(this))->_CheckForSource();
	return (mFormat != Format::Invalid);
}

//============================================================================================================
// Saves the image's color data into the specified memory buffer
//============================================================================================================

bool GLTexture::GetBuffer (Memory& mem)
{
	if (IsValid())
	{
		uint tex = GetTextureID();

		if (tex != 0 && mGlType == GL_TEXTURE_2D)
		{
			uint bpp = GetBitsPerPixel(mFormat);
			uint memSize = (bpp / 8) * mSize.x * mSize.y;

			if (memSize > 0)
			{
				uint type = (mFormat >= ITexture::Format::Float) ? GL_FLOAT : GL_UNSIGNED_BYTE;

				// Bind the current texture
				_BindTexture(mGlType, mGlID);

				// Read back the texture information
				glGetTexImage(GL_TEXTURE_2D, 0, _GetGLFormat(mFormat), type, mem.Resize(memSize));
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================
// Changes the texture wrapping mode
//============================================================================================================

void GLTexture::SetWrapMode (uint wrapMode)
{
	if (mWrapMode != wrapMode)
	{
		mWrapMode = wrapMode;
		if (mTex[0].GetSource().IsValid()) mSerializable = true;
	}
}

//============================================================================================================
// Changes the texture's filtering
//============================================================================================================

void GLTexture::SetFiltering (uint filtering)
{
	if (mFilter != filtering)
	{
		mFilter = filtering;
		if ( (mFilter & Filter::Mipmap) == 0 ) InvalidateMipmap();
		if (mTex[0].GetSource().IsValid()) mSerializable = true;
	}
}

//============================================================================================================
// Load a single texture from the specified file
//============================================================================================================

bool GLTexture::Load ( const String& file, uint format )
{
	mLock.Lock();

	// Release everything
	_InternalRelease(true);	

	// Try to load the file
	bool retVal = mTex[0].Load(file);

	if (retVal)
	{
		mRequestedFormat = format;

		mSize.x	= mTex[0].GetWidth();
		mSize.y	= mTex[0].GetHeight();
		mDepth	= mTex[0].GetDepth();
		mType	= (mDepth > 1) ? Type::ThreeDimensional : Type::TwoDimensional;
		mGlType	= (mDepth > 1) ? GL_TEXTURE_3D : GL_TEXTURE_2D;
		mFormat	= _GetSupportedFormat((format == Format::Optimal) ? mTex[0].GetFormat() : format);
	}

	mCheckForSource = false;
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Load a cube map using the six specified textures
//============================================================================================================

bool GLTexture::Load (	const String&	up,
						const String&	down,
						const String&	north,
						const String&	east,
						const String&	south,
						const String&	west,
						uint			format )
{
	mLock.Lock();
	_InternalRelease(true);	

	bool retVal = false;

	// Load all six textures
	if ( mTex[0].Load(east ) &&
		 mTex[1].Load(west ) &&
		 mTex[2].Load(north) &&
		 mTex[3].Load(south) &&
		 mTex[4].Load(up   ) &&
		 mTex[5].Load(down ) )
	{
		// Check their widths and heights -- they all must have equal values
		mSize.x = mTex[0].GetWidth();
		mSize.y = mTex[0].GetHeight();
		mDepth  = mTex[0].GetDepth();

		if ( mSize.x == mSize.y && mDepth == 1 )
		{
			retVal = true;

			for (uint i = 1; i < 6; ++i)
			{
				// If the values don't match, the cube map isn't usable
				if ( mTex[i].GetWidth()  != mSize.x ||
					 mTex[i].GetHeight() != mSize.y ||
					 mTex[i].GetDepth()  != 1 )
				{
					// Width and height don't match, and it isn't the first texture... so nothing to do.
					String debug;
					debug.Set( "Width and height parameters didn't match on a potential cube map texture '%s'", mName.GetBuffer() );
					WARNING( debug.GetBuffer() );
					retVal = false;
					break;
				}
			}
		}
	}

	if (retVal)
	{
		mRequestedFormat = format;

		// Success -- set the texture format before exiting
		mType	= Type::EnvironmentCubeMap;
		mGlType = GL_TEXTURE_CUBE_MAP;
		mFormat = _GetSupportedFormat((format == Format::Optimal) ? mTex[0].GetFormat() : format);
	}
	else
	{
		// Something went wrong
		_InternalRelease(true);
	}

	mCheckForSource = false;
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Assign texture data manually
//============================================================================================================

bool GLTexture::Set (const void*	buffer,
					 uint			width,
					 uint			height,
					 uint			depth,
					 uint			dataFormat,
					 uint			format)
{
	bool retVal (false);

	mLock.Lock();
	{
		_InternalRelease(true);

		if (buffer != 0)
		{
			mSize.x = width;
			mSize.y = height;
			mDepth = depth;

			// Special case for float to RGBA conversion -- automatically pack float into RGBA
			if (dataFormat == Format::Float && format == Format::RGBA)
			{
				// The data is changing from float to RGBA
				dataFormat = Format::RGBA;

				float* from = (float*)buffer;
				Color4ub* to = (Color4ub*)mTex[0].Reserve(width, height, depth, dataFormat);

				if (to != 0)
				{
					retVal = true;
					uint size = width * height;
					for (uint i = 0; i < size; ++i)
						to[i].PackFloat(from[i]);
				}

				// Encoded textures cannot be filtered
				mFilter = Filter::Nearest;
			}
			else
			{
				void* ptr = mTex[0].Reserve(width, height, depth, dataFormat);

				if (ptr != 0)
				{
					memcpy(ptr, buffer, mTex[0].GetSize());
					retVal = true;
				}
			}
		}
		else
		{
			static uint maxSize = glGetInteger(GL_MAX_TEXTURE_SIZE);

			if (width == 0 || height == 0)
			{
				ASSERT(false, "Requested texture dimensions are invalid!");
			}
			else if (width > maxSize || height > maxSize || depth > maxSize)
			{
				ASSERT(false, "Requested texture dimensions exceed your videocard's capabilities!");
			}
			else
			{
				mSize.Set(width, height);
				mDepth = depth;
				retVal = true;
			}
		}

		if (retVal)
		{
			mCheckForSource  = false;
			mRequestedFormat = format;

			mType	= (mDepth > 1) ? Type::ThreeDimensional : Type::TwoDimensional;
			mFormat = _GetSupportedFormat((format == Format::Optimal) ? dataFormat : format);
		}
	}
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Serialization -- saving
//============================================================================================================

bool GLTexture::SerializeTo (TreeNode& root) const
{
	if (mFormat == ITexture::Format::Invalid || mName.IsEmpty() || mTex[0].GetSource().IsEmpty() || !mSerializable)
		return false;

	mLock.Lock();
	{
		TreeNode& node = root.AddChild(ITexture::ClassID(), mName);

		if (mType == Type::EnvironmentCubeMap)
		{
			node.AddChild("Positive X", mTex[0].GetSource());
			node.AddChild("Negative X", mTex[1].GetSource());
			node.AddChild("Positive Y", mTex[2].GetSource());
			node.AddChild("Negative Y", mTex[3].GetSource());
			node.AddChild("Positive Z", mTex[4].GetSource());
			node.AddChild("Negative Z", mTex[5].GetSource());
		}
		else
		{
			node.AddChild("Source", mTex[0].GetSource());
		}

		node.AddChild("Format", ITexture::FormatToString(mRequestedFormat));
		node.AddChild("Filtering", ITexture::FilterToString(mFilter));
		node.AddChild("Wrap Mode", ITexture::WrapModeToString(mWrapMode));
	}
	mLock.Unlock();
	return true;
}

//============================================================================================================
// Serialization -- loading
//============================================================================================================

bool GLTexture::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	// If the texture has already been created, don't bother
	if (mGlID != 0 && !forceUpdate) return true;

	mLock.Lock();
	{
		// The texture is serializable by default if we're here
		mSerializable = true;

		String file[6];
		uint format (mFormat);

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if		(tag == "Source")		value >> file[0];
			else if (tag == "Positive X")	value >> file[0];	// GL_TEXTURE_CUBE_MAP_POSITIVE_X
			else if (tag == "Negative X")	value >> file[1];	// GL_TEXTURE_CUBE_MAP_NEGATIVE_X
			else if (tag == "Positive Y")	value >> file[2];	// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
			else if (tag == "Negative Y")	value >> file[3];	// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
			else if (tag == "Positive Z")	value >> file[4];	// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
			else if (tag == "Negative Z")	value >> file[5];	// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
			else if (tag == "Serializable") value >> mSerializable;
			else if (value.IsString())
			{
				const String& s = value.AsString();
				if		(tag == "Format")		format		= ITexture::StringToFormat(s);
				else if (tag == "Filtering")	mFilter		= ITexture::StringToFilter(s);
				else if (tag == "Wrap Mode")	mWrapMode	= ITexture::StringToWrapMode(s);
			}
		}

		uint validCount = 0;

		// Try to load all specified textures
		for (uint i = 0; i < 6; ++i)
		{
			Image& img ( mTex[i] );

			if ( img.IsValid() )
			{
				++validCount;
			}
			else if ( file[i].IsValid() )
			{
				if ( img.Load(file[i]) )
				{
					// Cube map textures must have identical width and height; first texture always counts
					if ( i == 0 || (img.GetWidth()  == mTex[0].GetWidth() &&
									img.GetHeight() == mTex[0].GetHeight() &&
									img.GetDepth()  == 1) )
					{
						++validCount;
					}
					else
					{
						// Width and height don't match, and it isn't the first texture... so nothing to do.
						String debug;
						debug.Set( "Width and height parameters didn't match on a potential cube map texture '%s'",
							mName.GetBuffer() );
						WARNING( debug.GetBuffer() );
						img.Release();
					}
				}
			}
		}

		// If there are valid textures to work with
		if ( validCount > 0 )
		{
			mCheckForSource  = false;
			mRequestedFormat = format;

			mSize.x = mTex[0].GetWidth();
			mSize.y = mTex[0].GetHeight();
			mDepth  = mTex[0].GetDepth();
			mFormat = _GetSupportedFormat((format == Format::Optimal) ? mTex[0].GetFormat() : format);

			if ( validCount == 6 )
			{
				mType = Type::EnvironmentCubeMap;
			}
			else if ( mDepth > 1 )
			{
				mType = Type::ThreeDimensional;
			}
			else
			{
				mType = Type::TwoDimensional;
			}
		}
	}
	mLock.Unlock();
	return (mFormat != Format::Invalid);
}