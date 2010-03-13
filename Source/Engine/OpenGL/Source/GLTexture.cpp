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
	case ITexture::Format::DXT1:			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
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
// Generates a mipmap level using the specified buffer
//============================================================================================================

template <typename DataType, typename SumType>
uint Generate2DMipmap (uint glType, const DataType* buffer, uint width, uint height,
					   int inFormat, int outFormat, uint dataType, uint channels, uint level)
{
	uint w = width  >> 1;
	uint h = height >> 1;

	if (w == 0) w = 1;
	if (h == 0) h = 1;

	Array<DataType> temp;
	temp.ExpandTo(w * h * channels);
	uint bl, br, tl, tr, idx;
	SumType sum;

	// Standard 2x2 box filter is used for downsampling
	for (uint y = 0; y < height; y += 2)
	{
		for (uint x = 0; x < width; x += 2)
		{
			bl = y * width + x;
			br = bl + 1;
			tl = bl + width;
			tr = br + width;

			if (x + 1 >= width)  { br = bl; tr = tl; }
			if (y + 1 >= height) { tl = bl; tr = br; }

			bl *= channels;
			br *= channels;
			tl *= channels;
			tr *= channels;

			idx = (x >> 1) + (y >> 1) * w;
			DataType* out = &temp[idx * channels];

			for (uint i = 0; i < channels; ++i)
			{
				sum = (SumType)buffer[bl+i] + buffer[br+i] + buffer[tl+i] + buffer[tr+i];
				out[i] = (DataType)(sum / 4);
			}
		}
	}

	buffer	= temp.GetBuffer();
	width	= w;
	height	= h;

	// Calculate the number of pixels
	uint pixels = width * height;

	// Upload the texture data to the videocard
	glTexImage2D(glType, level, outFormat, width, height, 0, inFormat, dataType, buffer);

	// Continue generating mipmaps until we reach 1x1
	if (w > 1 || h > 1)
	{
		pixels += Generate2DMipmap<DataType, SumType>(glType, buffer, width, height,
			inFormat, outFormat, dataType, channels, ++level);
	}
	return pixels;
}

//============================================================================================================
// Counts the number of pixels used by the mip-mapped texture
//============================================================================================================

uint CountMipmapSize (uint width, uint height)
{
	if (width  == 0) width  = 1;
	if (height == 0) height = 1;

	uint pixels = width * height;

	return (pixels == 1) ? pixels : pixels + CountMipmapSize(width >> 1, height >> 1);
}

//============================================================================================================
// Macro that shortens the code, used in Create2DImage() function below
//============================================================================================================

#define GEN_2D_MIPMAP(type, sum, channels) Generate2DMipmap<type, sum>(glType, (const type*)buffer, \
	width, height, inFormat, outFormat, dataType, channels, 1)

//============================================================================================================
// Generates 2D mipmaps for the current texture
//============================================================================================================

uint Create2DMipmaps (uint glType, const void* buffer, uint width, uint height, int inFormat, int outFormat, uint dataType)
{
	uint pixels = width * height;

	if (dataType == GL_FLOAT)
	{
		if		(inFormat == GL_RGBA)				pixels += GEN_2D_MIPMAP(float, float, 4);
		else if (inFormat == GL_RGB)				pixels += GEN_2D_MIPMAP(float, float, 3);
		else if (inFormat == GL_LUMINANCE_ALPHA)	pixels += GEN_2D_MIPMAP(float, float, 2);
		else if (inFormat == GL_INTENSITY)			pixels += GEN_2D_MIPMAP(float, float, 1);
		else
		{
			// Unsupported format -- let the videocard handle it
			glGenerateMipmap(glType);
			return CountMipmapSize(width, height);
		}
	}
	else
	{
		if		(inFormat == GL_RGBA)				pixels += GEN_2D_MIPMAP(byte, uint, 4);
		else if (inFormat == GL_RGB)				pixels += GEN_2D_MIPMAP(byte, uint, 3);
		else if (inFormat == GL_LUMINANCE_ALPHA)	pixels += GEN_2D_MIPMAP(byte, uint, 2);
		else if (inFormat == GL_INTENSITY)			pixels += GEN_2D_MIPMAP(byte, uint, 1);
		else
		{
			// Unsupported format -- let the videocard handle it
			glGenerateMipmap(glType);
			return CountMipmapSize(width, height);
		}
	}
	return pixels;
}

//============================================================================================================
// Creates an OpenGL texture with all the specified parameters
//============================================================================================================

inline uint Create2DImage (uint glType, const void* buffer, uint width, uint height, int inFormat,
					int outFormat, uint dataType, bool mipmap)
{
	// Upload the starting image
	glTexImage2D(glType, 0, outFormat, width, height, 0, inFormat, dataType, buffer);

	// Non-compressed texture: use the videocard's accelerated functionality
	if (outFormat != GL_COMPRESSED_RGB_S3TC_DXT1_EXT  &&
		outFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT &&
		outFormat != GL_COMPRESSED_RGBA_S3TC_DXT3_EXT &&
		outFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		glGenerateMipmap(glType);
		return CountMipmapSize(width, height);
	}

	// Compressed texture mipmap generation is not always supported on all videocards
	return mipmap ? Create2DMipmaps(glType, buffer, width, height, inFormat, outFormat, dataType) : width * height;
}

//============================================================================================================
// All textures need to be created with a name
//============================================================================================================

GLTexture::GLTexture (const String& name, IGraphics* graphics) :
	mName				(name),
	mGraphics			(graphics),
	mCheckForSource		(true),
	mType				(Type::Invalid),
	mGlID				(0),
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
		mType	= (mDepth > 1) ? Type::ThreeDimensional : Type::TwoDimensional;

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
			if (Thread::GetID() == ((GLGraphics*)mGraphics)->GetThreadID())
			{
				DeleteTexture((void*)mGlID);
			}
			else
			{
				mGraphics->ExecuteBeforeNextFrame(DeleteTexture, (void*)mGlID);
			}
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

	// Depth and 3D textures should not be filtered
	if ((mFormat & ITexture::Format::Depth) != 0 || mGlType == GL_TEXTURE_3D)
	{
		mFilter = Filter::Nearest;
	}

	// ATI drivers seem to like it when the texture filtering is set prior to texture data
	mActiveFilter = mFilter;

	glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GetGLMinFilter());
	glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GetGLMagFilter());
	CHECK_GL_ERROR;

	// Figure out the appropriate texture format
	uint dataFormat = mTex[0].GetFormat();

	// We will not need to generate the mipmap after this as it's part of the creation process
	mRegenMipmap = false;

	// Start with an unsigned byte data type
	mDataType = GL_UNSIGNED_BYTE;

	if (mTex[0].GetBuffer() == 0)
	{
		if (mFormat == Format::Depth)
		{
			mInFormat = GL_DEPTH_COMPONENT;
		}
		else if (mFormat == Format::DepthStencil)
		{
			mInFormat = GL_DEPTH_STENCIL;
			mDataType = GL_UNSIGNED_INT_24_8;
		}
		else mInFormat = GL_RGB;
	}
	else
	{
		switch (dataFormat)
		{
			case Format::Alpha:			mInFormat = GL_LUMINANCE;		mDataType = GL_UNSIGNED_BYTE;	break;
			case Format::Luminance:		mInFormat = GL_LUMINANCE_ALPHA;	mDataType = GL_UNSIGNED_BYTE;	break;
			case Format::RGB:			mInFormat = GL_RGB;				mDataType = GL_UNSIGNED_BYTE;	break;
			case Format::RGBA:			mInFormat = GL_RGBA;			mDataType = GL_UNSIGNED_BYTE;	break;
			case Format::Float:			mInFormat = GL_LUMINANCE;		mDataType = GL_FLOAT;			break;
			case Format::RGB32F:		mInFormat = GL_RGB;				mDataType = GL_FLOAT;			break;
			case Format::RGBA32F:		mInFormat = GL_RGBA;			mDataType = GL_FLOAT;			break;
			default:
				ASSERT(false, "Invalid texture format");
				break;
		};
	}

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

	// Figure out the appropriate bitrate and OpenGL format
	uint bpp = ITexture::GetBitsPerPixel(mFormat) / 8;
	int outFormat = _GetGLFormat(mFormat);
	mSizeInMemory = 0;

	if (mGlType == GL_TEXTURE_2D)
	{
		if (mFormat == Format::DXTN)
		{
			// DXT5 format meant for normal maps -- blue color must be stored in the alpha channel
			uint size = mTex[0].GetSize();
			byte* buffer = (byte*)(mTex[0].GetBuffer());

			if (dataFormat == Format::RGB)
			{
				mInFormat = GL_RGBA;

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
					mSizeInMemory = bpp * Create2DImage(mGlType, data, mSize.x, mSize.y, mInFormat,
						outFormat, mDataType, (mFilter & Filter::Mipmap) != 0);
				}
				delete [] data;
			}
			else if (dataFormat == Format::RGBA)
			{
				// Swap B and A components
				for (uint i = 0; i < size; i+=4) Swap(buffer[i+2], buffer[i+3]);

				mSizeInMemory = bpp * Create2DImage(mGlType, buffer, mSize.x, mSize.y, mInFormat,
					outFormat, mDataType, (mFilter & Filter::Mipmap) != 0);
			}
			else
			{
				ASSERT(false, "Unsupported data format for DXTN compression. You must use RGB or RGBA for input data.");
				mSizeInMemory = bpp * Create2DImage(mGlType, buffer, mSize.x, mSize.y, mInFormat,
					outFormat, mDataType, (mFilter & Filter::Mipmap) != 0);
			}
		}
		else
		{
			// Basic 2D texture has a single image source
			mSizeInMemory = bpp * Create2DImage(mGlType, mTex[0].GetBuffer(), mSize.x, mSize.y, mInFormat,
				outFormat, mDataType, (mFilter & Filter::Mipmap) != 0);
		}
	}
	else if ( mGlType == GL_TEXTURE_CUBE_MAP )
	{
		// Cubemap texture has 6 source images
		for (uint i = 0; i < 6; ++i)
		{
			mSizeInMemory += bpp * Create2DImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mTex[i].GetBuffer(),
				mSize.x, mSize.y, mInFormat, outFormat, mDataType, (mFilter & Filter::Mipmap) != 0);
		}
	}
	else if ( mGlType == GL_TEXTURE_3D )
	{
		// 3D texture
		glTexImage3D(mGlType, 0, outFormat, mSize.x, mSize.y, mDepth, 0, mInFormat, mDataType, mTex[0].GetBuffer());
		mSizeInMemory = bpp * mSize.x * mSize.y * mDepth;
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

	// Keep track of video memory used by textures
	g_caps.IncreaseTextureMemory(mSizeInMemory);

#ifdef _DEBUG
												System::Log("[TEXTURE] Created '%s'", mName.GetBuffer() );
	if		(mGlType == GL_TEXTURE_2D)			System::Log( "          - Dims:     %u x %u",		mSize.x, mSize.y );
	else if (mGlType == GL_TEXTURE_CUBE_MAP)	System::Log( "          - Dims:     %u x %u x 6",	mSize.x, mSize.y );
	else if (mGlType == GL_TEXTURE_3D)			System::Log( "          - Dims:     %u x %u x %u",	mSize.x, mSize.y, mDepth );

	System::Log("          - Format:   %s", ITexture::FormatToString(mFormat));
	System::Log("          - Size:     %s bytes", String::GetFormattedSize(mSizeInMemory).GetBuffer());
#endif

	// Release the memory
	for (uint i = 0; i < 6; ++i) mTex[i].Release();

	mLock.Unlock();
	CHECK_GL_ERROR;
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
		// Adjust the filtering if necessary
		if (mActiveFilter != mFilter)
		{
			mActiveFilter = mFilter;

			if (!active)
			{
				active = true;
				_BindTexture( mGlType, mGlID );
			}

			glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GetGLMinFilter());
			glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, GetGLMagFilter());
			CHECK_GL_ERROR;
		}

		// Anisotropy is only enabled for mip-mapped textures
		{
			uint af = (mFilter & ITexture::Filter::Mipmap) ? mDefaultAF : 0;

			// Adjust the active texture anisotropy level
			if (mActiveAF != af)
			{
				if (af > 1)
				{
					if (!active)
					{
						active = true;
						_BindTexture( mGlType, mGlID );
					}

					glTexParameteri(mGlType, GL_TEXTURE_MAX_ANISOTROPY_EXT, mActiveAF = af);
					CHECK_GL_ERROR;
				}
				else
				{
					glTexParameteri(mGlType, GL_TEXTURE_MAX_ANISOTROPY_EXT, mActiveAF = 0);
					CHECK_GL_ERROR;
				}
			}
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

		if (mRegenMipmap && mGlType == GL_TEXTURE_2D)
		{
			mRegenMipmap = false;

			if (!active)
			{
				active = true;
				_BindTexture( mGlType, mGlID );
			}

			// Just in case, for ATI
			glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

			g_caps.DecreaseTextureMemory(mSizeInMemory);
			{
				if ((mFormat & ITexture::Format::Compressed) == 0)
				{
					// Non-compressed texture: use the videocard's accelerated functionality
					glGenerateMipmap(mGlType);
					mSizeInMemory = CountMipmapSize(mSize.x, mSize.y);
				}
				else
				{
					// Compressed texture mipmap generation is not always supported on all videocards
					uint type = GL_UNSIGNED_BYTE, memSize = 4 * mSize.x * mSize.y;

					if (mFormat >= ITexture::Format::Float)
					{
						type = GL_FLOAT;
						memSize *= 4;
					}

					// Read back the texture information
					Memory mem;
					glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, type, mem.Resize(memSize));

					// Create the mipmaps using the texture information
					mSizeInMemory = Create2DMipmaps(mGlType, mem.GetBuffer(), mSize.x, mSize.y, GL_RGBA,
						_GetGLFormat(mFormat), mDataType);
				}

				// Take the number of bytes per pixel into account
				mSizeInMemory *= ITexture::GetBitsPerPixel(mFormat) / 8;
			}
			g_caps.IncreaseTextureMemory(mSizeInMemory);
		}
	}
	mTimestamp = Time::GetMilliseconds();
	return mGlID;
}

//============================================================================================================
// Binds the specified texture
//============================================================================================================

inline bool GLTexture::_BindTexture (uint glType, uint glID)
{
	return ((GLGraphics*)mGraphics)->_BindTexture(glType, glID);
}

//============================================================================================================
// OpenGL minification filter
//============================================================================================================

inline uint GLTexture::GetGLMinFilter() const
{
	return ((mFilter & Filter::Mipmap) != 0) ? GL_LINEAR_MIPMAP_NEAREST :
		(mFilter == Filter::Linear ? GL_LINEAR : GL_NEAREST);
}

//============================================================================================================
// OpenGL magnification filter
//============================================================================================================

inline uint GLTexture::GetGLMagFilter() const
{
	return (mFilter == Filter::Nearest) ? GL_NEAREST : GL_LINEAR;
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
		if (mTex[0].GetSource().IsValid()) mSerializable = true;
	}
}

//============================================================================================================
// Load a single texture from the specified file
//============================================================================================================

bool GLTexture::Load (const String& file, uint format)
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
		uint wrapMode	= mWrapMode;
		uint filter		= mFilter;

		_InternalRelease(true);

		mWrapMode	= wrapMode;
		mFilter		= filter;

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