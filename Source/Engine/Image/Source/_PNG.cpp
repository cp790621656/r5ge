#include "../Include/_All.h"
#include "../Include/_Codecs.h"
#include "../Include/png/png.h"
#include <setjmp.h>

#ifdef _WINDOWS
  #pragma comment(lib, "png.lib")
  #pragma comment(lib, "zlib.lib")
#endif

namespace R5
{
namespace Codec
{

//========================================================================================================
// Struct used to pass the buffer information to OnRead function below
//========================================================================================================

struct IOBuffer
{
	const byte* mBuffer;
	uint		 mSize;
};

//========================================================================================================
// Overriding function: Warning callback
//========================================================================================================

static void OnWarning (png_structp png_ptr, png_const_charp message)
{
	WARNING(message);
}

//========================================================================================================
// Overriding function: Error callback
//========================================================================================================

static void OnError (png_structp png_ptr, png_const_charp message)
{
	ASSERT(false, message);
	longjmp (png_jmpbuf(png_ptr), 1);
}

//========================================================================================================
// Overriding function: Read callback
//========================================================================================================

static void OnRead (png_structp pngStruct, png_bytep data, png_size_t length)
{
	IOBuffer* buffer = (IOBuffer*)pngStruct->io_ptr;
	ASSERT(buffer != 0, "io_ptr is empty");

	if (length > buffer->mSize) length = buffer->mSize;
	memcpy(data, buffer->mBuffer, length);

	buffer->mBuffer += length;
	buffer->mSize   -= length;
}

//========================================================================================================
// Portable Network Graphics format codec (.png)
//========================================================================================================

R5_IMAGE_CODEC(PNG)
{
	// Ensure that the header is what's expected
	uint header = *(const uint*)buff;

	if (header == 0x474E5089)
	{
		png_structp pngStruct	= png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, OnError, OnWarning);
		png_infop	pngInfo		= png_create_info_struct(pngStruct);

		if ( setjmp(png_jmpbuf(pngStruct)) )
		{
			png_destroy_read_struct(&pngStruct, &pngInfo, 0);
			return false;
		}

		IOBuffer b;
		b.mBuffer = buff;
		b.mSize = size;

		png_set_read_fn	(pngStruct, &b, OnRead);
		png_set_error_fn(pngStruct, &b, OnError, OnWarning);
		png_read_info	(pngStruct, pngInfo);

		// Get the header information
		int	depth(0), type(0);
		png_get_IHDR(pngStruct, pngInfo, 0, 0, &depth, &type, 0, 0, 0);

		// Expand low-bit-depth grayscale images to 8 bits
		if (type == PNG_COLOR_TYPE_GRAY && depth < 8)
			png_set_gray_1_2_4_to_8(pngStruct);

		// Convert palettes to RGB
		if (png_get_valid(pngStruct, pngInfo, PNG_INFO_PLTE))
			png_set_palette_to_rgb(pngStruct);

		// Expand RGB images with transparency to full alpha channels
		if (png_get_valid(pngStruct, pngInfo, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(pngStruct);

		// Refresh the header information
		png_get_IHDR(pngStruct, pngInfo, 0, 0, &depth, &type, 0, 0, 0);

		// Expand palette and RGB images
		if (depth < 8)
		{
			depth = 8;
			png_set_packing(pngStruct);
		}

		// Update the info structure
		png_read_update_info(pngStruct, pngInfo);

		// Get the final information
		uint width	  = png_get_image_width	(pngStruct, pngInfo);
		uint height	  = png_get_image_height(pngStruct, pngInfo);
		uint channels = png_get_channels	(pngStruct, pngInfo);

		// Determine the format
		if		(channels == 1)	out.mFormat = ITexture::Format::Alpha;
		else if (channels == 2)	out.mFormat = ITexture::Format::Luminance;
		else if (channels == 3)	out.mFormat = ITexture::Format::RGB;
		else if (channels == 4)	out.mFormat = ITexture::Format::RGBA;
		else
		{
			// Invalid -- possibly a color palette
			WARNING("Unsupported PNG format");
			png_destroy_read_struct(&pngStruct, &pngInfo, 0);
			return false;
		}

		// Get the width and height and reserve the buffer
		out.mWidth  = width;
		out.mHeight = height;
		out.mDepth  = 1;

		// Figure out the number of bytes per line and reserve the memory for the image
		uint bytesPerLine = width * channels;
		byte* buffer = out.mBytes.Resize(bytesPerLine * height);

		// PNG library's read function needs an array of pointers to individual lines
		png_bytepp rows = new png_bytep[height * sizeof(png_bytep)];

		// Copy offset pointers within the buffer, flipping it in the process
		for (uint i = 0; i < height; ++i)
			rows[height - 1 - i] = buffer + i * bytesPerLine;

		// Read the image
		png_read_image(pngStruct, rows);

		// Cleanup
		delete [] rows;
		png_destroy_read_struct(&pngStruct, &pngInfo, 0);
		return true;
	}
	return false;
}

}; // namespace Codec
}; // namespace R5