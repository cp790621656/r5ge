#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Image loading library
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

	// Codec functions accept the input data buffer, size of the buffer, and the output image buffer
	typedef FastDelegate<bool (const byte* in, uint inSize, const String& extension, Image::Buffer& out)> CodecDelegate;

	// STATIC: Registeres a new codec
	static void RegisterCodec (const String& name, const CodecDelegate& fnct);

	// STATIC: Retrieves the names of all registered codecs
	static void GetRegisteredCodecs (Array<String>& out);

	struct Utilities
	{
		// STATIC: Creates a normal map for the specified heightmap
		static void HeightMapToNormalMap (	const float*		buffer,
											uint				width,
											uint				height,
											Array<Color4ub>&	c,
											bool				seamless,
											const Vector3f&		scale = Vector3f(1.0f, 1.0f, 0.1f) );
	};

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
};