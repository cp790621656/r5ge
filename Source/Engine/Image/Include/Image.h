#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Image loading library
// Author: Michael Lyashenko
//============================================================================================================

class Image
{
public:

	// Image format is the same as the texture format
	typedef R5::ITexture::Format Format;

	// Image buffer holds a memory buffer, its width, height, and format
	struct Buffer
	{
		Memory	mBytes;
		uint	mWidth;
		uint	mHeight;
		uint	mDepth;
		uint	mFormat;

		Buffer() : mWidth(0), mHeight(0), mDepth(0), mFormat(Image::Format::Invalid) {}

		bool IsValid() const { return (mFormat != Image::Format::Invalid) && (mBytes.GetSize() > 0); }

		void Release()
		{
			mBytes.Release();
			mWidth  = 0;
			mHeight = 0;
			mDepth  = 0;
			mFormat = Image::Format::Invalid;
		}
	};

protected:

	String	mSource;	// Actual filename from which the image has been loaded
	String	mLoadingFN;	// Temporary filename, internal use
	Buffer	mBuffer;	// Actual image buffer

public:

	// Read codec functions accept the input data buffer, size of the buffer, and the output image buffer
	typedef FastDelegate<bool (const byte* in, uint inSize, const String& extension, Image::Buffer& out)> ReadDelegate;

	// Write codec functions accept the memory buffer to write to as well as the image data to encode
	typedef FastDelegate<bool (Memory& out, const byte* buffer, uint width, uint height, uint format)> WriteDelegate;

	// STATIC: Registeres a new codec
	static void RegisterCodec (const String& name, const ReadDelegate& read, const WriteDelegate& write);

	// STATIC: Retrieves the names of all registered codecs
	static void GetRegisteredCodecs (Array<String>& out);

	// STATIC: Creates a normal map for the specified heightmap
	static void HeightMapToNormalMap (	const float*		buffer,
										uint				width,
										uint				height,
										Array<Color4ub>&	c,
										bool				seamless,
										const Vector3f&		scale = Vector3f(1.0f, 1.0f, 0.1f) );

	// STATIC: Saves the specified texture buffer into the specified memory buffer as if saving to a file
	static bool StaticSave (Memory& out, const String& extension, const byte* buff, uint width, uint height, uint format);

	// STATIC: Saves the specified texture buffer into the specified file
	static bool StaticSave (const String& file, const byte* buff, uint width, uint height, uint format);

public:

	~Image() { Release(); }

	const String&	GetSource()	const { return mSource; }
	const void*		GetBuffer()	const { return mBuffer.mBytes.GetBuffer(); }
	
	uint GetSize()	 const { return mBuffer.mBytes.GetSize();   }
	uint GetWidth()	 const { return mBuffer.mWidth;    }
	uint GetHeight() const { return mBuffer.mHeight;   }
	uint GetDepth()	 const { return mBuffer.mDepth;    }
	uint GetFormat() const { return mBuffer.mFormat;   }
	bool IsValid()	 const { return mBuffer.IsValid(); }
	void Release();

	// Allocates a buffer large enough to store the image of specified width, height, and format
	void* Reserve (uint width, uint height, uint depth, uint format);

	// Load texture information from the specified file
	bool Load (const String& file);

	// Decode a previously loaded buffer
	bool Load (const void* buffer, uint size);

	// Saves the texture into the specified file
	bool Save (const String& file) const
	{
		return StaticSave(file, (const byte*)GetBuffer(), GetWidth(), GetHeight(), GetFormat());
	}

	// Saves the texture into the specified memory buffer
	bool Save (Memory& mem, const String& extension)
	{
		return StaticSave(mem, extension, (const byte*)GetBuffer(), GetWidth(), GetHeight(), GetFormat());
	}
};