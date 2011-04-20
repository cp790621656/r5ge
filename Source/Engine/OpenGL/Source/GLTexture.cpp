#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Helper struct used to convert texture types from R5 to OpenGL
//============================================================================================================

const uint convertTextureTypeToGL [5] =
{
	0,
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP,
};

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

void DeleteTexture (IGraphicsManager* graphics, void* ptr)
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
	CHECK_GL_ERROR;

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
	uint pixels = 0;
	
	// Compressed textures and Intel cards should always go through manual mipmap generation
	if (
#ifdef _WINDOWS
		glGenerateMipmap == 0 ||
#endif
		outFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT  ||
		outFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
		outFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
		outFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ||
		g_caps.mVendor == IGraphics::DeviceInfo::Vendor::Intel)
	{
		// Manual mipmap generation
		pixels = width * height;
		
		if (dataType == GL_FLOAT)
		{
			if		(inFormat == GL_RGBA)				pixels += GEN_2D_MIPMAP(float, float, 4);
			else if (inFormat == GL_RGB)				pixels += GEN_2D_MIPMAP(float, float, 3);
			else if (inFormat == GL_LUMINANCE_ALPHA)	pixels += GEN_2D_MIPMAP(float, float, 2);
			else if (inFormat == GL_INTENSITY)			pixels += GEN_2D_MIPMAP(float, float, 1);
		}
		else
		{
			if		(inFormat == GL_RGBA)				pixels += GEN_2D_MIPMAP(byte, uint, 4);
			else if (inFormat == GL_RGB)				pixels += GEN_2D_MIPMAP(byte, uint, 3);
			else if (inFormat == GL_LUMINANCE_ALPHA)	pixels += GEN_2D_MIPMAP(byte, uint, 2);
			else if (inFormat == GL_INTENSITY)			pixels += GEN_2D_MIPMAP(byte, uint, 1);
		}
	}
	else
	{
		// Hardware-accelerated mipmap generation
		glGenerateMipmap(glType);
		CHECK_GL_ERROR;
		pixels = CountMipmapSize(width, height);
	}
	return pixels;
}

//============================================================================================================
// Create an OpenGL texture with all the specified parameters
//============================================================================================================

inline uint Create2DImage (uint glType, const void* buffer, uint width, uint height, int inFormat,
	int outFormat, uint dataType, bool mipmap)
{
	// Upload the starting image
	glTexImage2D(glType, 0, outFormat, width, height, 0, inFormat, dataType, buffer);
	CHECK_GL_ERROR;
	return (mipmap) ? Create2DMipmaps(glType, buffer, width, height, inFormat, outFormat, dataType) : width * height;
}

//============================================================================================================
// All textures need to be created with a name
//============================================================================================================

GLTexture::GLTexture (const String& name, IGraphics* graphics) :
	mName				(name),
	mGraphics			((GLGraphics*)graphics),
	mReplacement		(0),
	mType				(Type::Invalid),
	mGlID				(0),
	mGlType				(0),
	mFormat				(Format::Invalid),
	mRequestedFormat	(Format::Invalid),
	mDepth				(0),
	mSizeInMemory		(0),
	mTimestamp			(Time::GetMilliseconds()),
	mWrapMode			(WrapMode::Default),
	mCompareMode		(CompareMode::Default),
	mFilter				(Filter::Nearest),
	mActiveWrapMode		(WrapMode::Default),
	mActiveFilter		(Filter::Default),
	mActiveCompareMode	(CompareMode::Default),
	mActiveAF			(0),
	mActiveMSAA			(0),
	mMipmapsGenerated	(false),
	mRegenMipmap		(false),
	mCheckForSource		(true),
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

		// As a convenience, if the texture's name contains the format, use it
		if (mRequestedFormat == Format::Optimal)
		{
			if		(source.Contains("_Alpha.") ||
					 source.Contains("_AO.") ||
					 source.Contains("_Glow."))			mRequestedFormat = Format::Alpha;
			else if (source.Contains("_Luminance."))	mRequestedFormat = Format::Luminance;
			else if (source.Contains("_RGBA.") ||
					 source.Contains("_NS."))			mRequestedFormat = Format::RGBA;
			else if (source.Contains("_RGB.") ||
					 source.Contains("_N."))			mRequestedFormat = Format::RGB;
			else if (source.Contains("_DXT3."))			mRequestedFormat = Format::DXT3;
			else if (source.Contains("_DXT5."))			mRequestedFormat = Format::DXT5;
			else if (mSize.x > 128 || mSize.y > 128)
			{
				// Use DXT compression unless specified otherwise
				if (source.EndsWith(".jpg")) mRequestedFormat = Format::DXT3;
				else mRequestedFormat = Format::DXT5;
			}
		}

		if (mRequestedFormat != Format::Optimal)
			mFormat = _GetSupportedFormat(mRequestedFormat);
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
			if (Thread::GetID() == mGraphics->GetThreadID())
			{
				DeleteTexture(mGraphics, (void*)mGlID);
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

	// Release all texture data
	for (uint i = 0; i < 6; ++i) mTex[i].Release();

	// Reset all parameters
	mType				= Type::Invalid;
	mGlType				= 0;
	mFormat				= Format::Invalid;
	mRequestedFormat	= Format::Invalid;
	mSize.x				= 0;
	mSize.y				= 0;
	mDepth				= 0;
	mSizeInMemory		= 0;
	//mWrapMode			= WrapMode::Default;		// Intentionally commented out
	//mFilter			= Filter::Default;
	//mCompareMode		= CompareMode::Default;
	mActiveWrapMode		= WrapMode::Default;
	mActiveFilter		= Filter::Default;
	mActiveCompareMode	= CompareMode::Default;
	mActiveAF			= 0;
	mActiveMSAA			= 0;
	mMipmapsGenerated	= false;
	mRegenMipmap		= false;
	mCheckForSource		= true;
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

	// Don't ask for antia-aliased textures if they're not supported
	if (!g_caps.mMSAA) mType = ITexture::Type::TwoDimensional;

	// Figure out the texture's OpenGL type
	ASSERT(mType <= ITexture::Type::EnvironmentCubeMap, "Invalid type");
	mGlType = convertTextureTypeToGL[mType];

	// Bind the texture
#ifdef GL_ARB_texture_multisample
	mGraphics->_BindTexture(mActiveMSAA ? GL_TEXTURE_2D_MULTISAMPLE : mGlType, mGlID);
#else
	mGraphics->_BindTexture(mGlType, mGlID);
#endif

	// Anti-aliased textures should be a bit more restrictive
	if (mActiveMSAA > 1)
	{
		mFilter = ITexture::Filter::Nearest;
		mWrapMode = ITexture::WrapMode::ClampToEdge;
	}

	bool isDepth = ((mFormat & ITexture::Format::Depth) != 0);
	bool is3D = (mGlType == GL_TEXTURE_3D);

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
	else if ( mFilter > Filter::Linear && (is3D || isDepth) )
	{
		// Depth and 3D textures should not be mip-mapped
		mFilter = Filter::Linear;
	}

	// ATI drivers seem to like it when the texture filtering is set prior to texture data
	mActiveFilter = mFilter;
	glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, _GetGLMinFilter());
	glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, _GetGLMagFilter());
	CHECK_GL_ERROR;

	// Figure out the appropriate texture format
	uint inDataFormat = mTex[0].GetFormat();

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
		switch (inDataFormat)
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

	// Figure out the appropriate bitrate and OpenGL format
	uint bpp = ITexture::GetBitsPerPixel(mFormat) >> 3;
	int outFormat = _GetGLFormat(mFormat);
	mSizeInMemory = 0;

#ifdef GL_ARB_texture_multisample
	if (mActiveMSAA > 1)
	{
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mActiveMSAA, outFormat, mSize.x, mSize.y, 0);
		CHECK_GL_ERROR;
		mSizeInMemory = bpp * mSize.x * mSize.y * mActiveMSAA;
	}
	else
#endif
	if (mGlType == GL_TEXTURE_2D)
	{
		if (mFormat == Format::DXTN)
		{
			// DXT5 format meant for normal maps -- blue color must be stored in the alpha channel
			uint size = mTex[0].GetSize();
			byte* buffer = (byte*)(mTex[0].GetBuffer());

			if (inDataFormat == Format::RGB)
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
			else if (inDataFormat == Format::RGBA)
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
	else if (mGlType == GL_TEXTURE_CUBE_MAP)
	{
		// Cubemap texture has 6 source images
		for (uint i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, outFormat, mSize.x, mSize.y, 0, mInFormat, mDataType, mTex[i].GetBuffer());
			CHECK_GL_ERROR;
		}
		
		if ((mFilter & Filter::Mipmap) != 0)
		{
			for (uint i = 0; i < 6; ++i)
			{
				mSizeInMemory += Create2DMipmaps(GL_TEXTURE_CUBE_MAP, mTex[i].GetBuffer(), mSize.x, mSize.y, mInFormat, outFormat, mDataType);
			}
		}
		else mSizeInMemory += mSize.x * mSize.y * 6;
	}
	else if (mGlType == GL_TEXTURE_3D)
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

	if (mActiveMSAA > 1)
	{
		System::Log( "          - Dims:     %u x %u (%u samples)",	mSize.x, mSize.y, mActiveMSAA );
	}
	else if	(mGlType == GL_TEXTURE_2D)
	{
		System::Log( "          - Dims:     %u x %u",		mSize.x, mSize.y );
	}
	else if (mGlType == GL_TEXTURE_CUBE_MAP)
	{
		System::Log( "          - Dims:     %u x %u x 6",	mSize.x, mSize.y );
	}
	else if (mGlType == GL_TEXTURE_3D)
	{
		System::Log( "          - Dims:     %u x %u x %u",	mSize.x, mSize.y, mDepth );
	}

	System::Log("          - Format:   %s", ITexture::FormatToString(mFormat));
	System::Log("          - Size:     %s bytes", String::GetFormattedSize(mSizeInMemory).GetBuffer());
#endif

	// Release the memory
	for (uint i = 0; i < 6; ++i) mTex[i].Release();

	mLock.Unlock();
	CHECK_GL_ERROR;
}

//============================================================================================================
// OpenGL minification filter
//============================================================================================================

inline uint GLTexture::_GetGLMinFilter() const
{
	return ((mFilter & Filter::Mipmap) != 0) ? GL_LINEAR_MIPMAP_NEAREST :
			(mFilter == Filter::Linear ? GL_LINEAR : GL_NEAREST);
}

//============================================================================================================
// OpenGL magnification filter
//============================================================================================================

inline uint GLTexture::_GetGLMagFilter() const
{
	return (mFilter == Filter::Nearest) ? GL_NEAREST : GL_LINEAR;
}

//============================================================================================================
// Whether the texture can be used
//============================================================================================================

bool GLTexture::IsValid() const
{
	if (mReplacement != 0) return mReplacement->IsValid();
	if (mCheckForSource) (const_cast<GLTexture*>(this))->_CheckForSource();
	return (mFormat != Format::Invalid);
}

//============================================================================================================
// Returns the valid path to the texture's source
//============================================================================================================

const String& GLTexture::GetSource (uint index) const
{
	if (mReplacement != 0) return mReplacement->GetSource(index);
	return (index < 6 ? mTex[index].GetSource() : mTex[0].GetSource());
}

//============================================================================================================
// Saves the image's color data into the specified memory buffer
//============================================================================================================

bool GLTexture::GetBuffer (Memory& mem)
{
	if (mReplacement != 0) return mReplacement->GetBuffer(mem);

	if (IsValid())
	{
		uint tex = Activate();

		if (tex != 0 && mGlType == GL_TEXTURE_2D && mActiveMSAA < 2)
		{
			uint bpp = GetBitsPerPixel(mFormat);
			uint memSize = (bpp / 8) * mSize.x * mSize.y;

			if (memSize > 0)
			{
				uint type = (mFormat >= ITexture::Format::Float) ? GL_FLOAT : GL_UNSIGNED_BYTE;

				// Bind the current texture
				mGraphics->_BindTexture(GL_TEXTURE_2D, mGlID);

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
	if (mReplacement != 0) return mReplacement->SetWrapMode(wrapMode);

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
	if (mReplacement != 0) return mReplacement->SetFiltering(filtering);

	if (mFilter != filtering)
	{
		mFilter = filtering;
		if (mTex[0].GetSource().IsValid()) mSerializable = true;
	}
}

//============================================================================================================
// Changes the texture's color compare mode
//============================================================================================================

void GLTexture::SetCompareMode (uint compareMode)
{
	if (mReplacement != 0) return mReplacement->SetCompareMode(compareMode);

	if (mCompareMode != compareMode)
	{
		mCompareMode = compareMode;
		if (mTex[0].GetSource().IsValid()) mSerializable = true;
	}
}

//============================================================================================================
// Activates the texture and returns its identifier
//============================================================================================================

uint GLTexture::Activate()
{
	if (mReplacement != 0) return mReplacement->Activate();

	// If OpenGL texture hasn't been created yet, and there's data present, let's create it
	if (mGlID == 0 && IsValid()) _Create();
#ifdef GL_ARB_texture_multisample
	else mGraphics->_BindTexture(mActiveMSAA ? GL_TEXTURE_2D_MULTISAMPLE : mGlType, mGlID);
#else
	else mGraphics->_BindTexture(mGlType, mGlID);
#endif

	if (mGlID != 0)
	{
		// Adjust the filtering if necessary
		if (mActiveFilter != mFilter)
		{
			mActiveFilter = mFilter;
			glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, _GetGLMinFilter());
			glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, _GetGLMagFilter());
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

		// Texture color compare mode
		if (mActiveCompareMode != mCompareMode)
		{
			mActiveCompareMode = mCompareMode;

			if (mCompareMode == CompareMode::Shadow)
			{
				// Compare depth
				glTexParameteri(mGlType, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
				glTexParameteri(mGlType, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
			}
			else
			{
				// No comparison
				glTexParameteri(mGlType, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
			}
			CHECK_GL_ERROR;
		}

		if (mRegenMipmap && mGlType == GL_TEXTURE_2D)
		{
			mRegenMipmap = false;

			// Just in case, for ATI
			glTexParameteri(mGlType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			CHECK_GL_ERROR;

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
					glGetTexImage(mGlType, 0, GL_RGBA, type, mem.Resize(memSize));
					CHECK_GL_ERROR;

					// Create the mipmaps using the texture information
					mSizeInMemory = Create2DMipmaps(mGlType, mem.GetBuffer(), mSize.x, mSize.y, GL_RGBA,
						_GetGLFormat(mFormat), mDataType);
				}

				// Take the number of bytes per pixel into account
				mSizeInMemory *= ITexture::GetBitsPerPixel(mFormat) >> 3;
			}
			g_caps.IncreaseTextureMemory(mSizeInMemory);
		}
	}
	mTimestamp = Time::GetMilliseconds();
	return mGlID;
}

//============================================================================================================
// Reserve an internal texture of specified dimensions
//============================================================================================================

bool GLTexture::Reserve (uint width, uint height, uint depth, uint format, uint msaa)
{
	if (mReplacement != 0) return Reserve(width, height, depth, format, msaa);

	mLock.Lock();
	{
		uint wrapMode	= mWrapMode;
		uint filter		= mFilter;

		_InternalRelease(true);

		mWrapMode	= wrapMode;
		mFilter		= filter;

		mSize.Set(width, height);
		mDepth = 1;

		if (depth > 1)
		{
			mType = Type::ThreeDimensional;
		}
		else
		{
			if (msaa > 0 && g_caps.mMSAA)
			{
				mActiveMSAA = (msaa > 8) ? 8 : msaa;
			}
			mType = Type::TwoDimensional;
		}

		// Choose the best format
		mFormat = _GetSupportedFormat((format == Format::Optimal) ? Format::RGBA : format);

		// Don't bother checking for source
		mCheckForSource  = false;
		mRequestedFormat = format;
	}
	mLock.Unlock();
	return true;
}

//============================================================================================================
// Load a single texture from the specified file
//============================================================================================================

bool GLTexture::Load (const String& file, uint format)
{
	if (mReplacement != 0) return mReplacement->Load(file, format);

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
	if (mReplacement != 0) return mReplacement->Load(up, down, north, east, south, west, format);

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
				if ( mTex[i].GetWidth()  != (ushort)mSize.x ||
					 mTex[i].GetHeight() != (ushort)mSize.y ||
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
	if (buffer == 0) return Reserve(width, height, depth, format, 0);
	if (mReplacement != 0) return mReplacement->Set(buffer, width, height, depth, dataFormat, format);

	bool retVal (false);

	mLock.Lock();
	{
		uint wrapMode	= mWrapMode;
		uint filter		= mFilter;

		_InternalRelease(true);

		mWrapMode	= wrapMode;
		mFilter		= filter;

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
// Assign the cube map data manually
//============================================================================================================

bool GLTexture::Set(const void*	up,
					const void*	down,
					const void*	north,
					const void*	east,
					const void*	south,
					const void*	west,
					uint width, uint height,
					uint dataFormat,
					uint format)
{
	if (mReplacement != 0)
	{
		return mReplacement->Set(up, down, north, east, south, west, width, height, dataFormat, format);
	}

	bool retVal (false);

	mLock.Lock();
	{
		uint wrapMode	= mWrapMode;
		uint filter		= mFilter;

		_InternalRelease(true);

		mWrapMode	= wrapMode;
		mFilter		= filter;

		if (up != 0 && down != 0 && north != 0 && east != 0 && south != 0 && west != 0)
		{
			mSize.x = width;
			mSize.y = height;
			mDepth = 1;

			void* p0 = mTex[0].Reserve(width, height, 1, dataFormat);
			void* p1 = mTex[1].Reserve(width, height, 1, dataFormat);
			void* p2 = mTex[2].Reserve(width, height, 1, dataFormat);
			void* p3 = mTex[3].Reserve(width, height, 1, dataFormat);
			void* p4 = mTex[4].Reserve(width, height, 1, dataFormat);
			void* p5 = mTex[5].Reserve(width, height, 1, dataFormat);

			if (p0 != 0 && p1 != 0 && p2 != 0 && p3 != 0 && p4 != 0 && p5 != 0)
			{
				memcpy(p0, east,	mTex[0].GetSize());
				memcpy(p1, west,	mTex[1].GetSize());
				memcpy(p2, north,	mTex[2].GetSize());
				memcpy(p3, south,	mTex[3].GetSize());
				memcpy(p4, up,		mTex[4].GetSize());
				memcpy(p5, down,	mTex[5].GetSize());

				retVal = true;
			}

			if (retVal)
			{
				mCheckForSource  = false;
				mRequestedFormat = format;

				mType	= Type::EnvironmentCubeMap;
				mFormat = _GetSupportedFormat((format == Format::Optimal) ? dataFormat : format);
			}
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
	if (mReplacement != 0) return mReplacement->SerializeTo(root);

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

		if (mCompareMode != CompareMode::Default)
		{
			node.AddChild("Compare Mode", ITexture::CompareModeToString(mCompareMode));
		}
	}
	mLock.Unlock();
	return true;
}

//============================================================================================================
// Serialization -- loading
//============================================================================================================

bool GLTexture::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	if (mReplacement != 0) return mReplacement->SerializeFrom(root, forceUpdate);

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
				if		(tag == "Format")		format			= ITexture::StringToFormat(s);
				else if (tag == "Filtering")	mFilter			= ITexture::StringToFilter(s);
				else if (tag == "Wrap Mode")	mWrapMode		= ITexture::StringToWrapMode(s);
				else if (tag == "Compare Mode")	mCompareMode	= ITexture::StringToCompareMode(s);
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
