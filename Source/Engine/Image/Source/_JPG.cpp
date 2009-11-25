#include "../Include/_All.h"
#include "../Include/_Codecs.h"
#include "../Include/jpeg/jpeglib.h"
#include <setjmp.h>

#ifdef _WINDOWS
  #pragma comment(lib, "jpeg.lib")
#endif

namespace R5
{
namespace Codec
{

//========================================================================================================
// Custom data passed in the custom field, used to stop parsing the file if an error is occured
//========================================================================================================

struct CustomData
{
	bool	 mError;
	jmp_buf	 mJump;
};

//========================================================================================================
// Overriding function: Initialize the source -- do nothing
//========================================================================================================

static void OnInitSource (j_decompress_ptr cinfo)
{
}

//========================================================================================================
// Overriding function: The buffer is filled at the very beginning, so this function is not needed
//========================================================================================================

static boolean OnFillBuffer (j_decompress_ptr cinfo)
{
	return 0;
}

//========================================================================================================
// Overriding function: Skip past some data
//========================================================================================================

static void OnSkipData (j_decompress_ptr cinfo, long numBytes)
{
	uint bytes = (uint)numBytes;

	// Ensure that the skipped number of bytes never goes past the end of the buffer
	if (cinfo->src->bytes_in_buffer < bytes) bytes = cinfo->src->bytes_in_buffer;

	// Skip the specified amount of bytes
	cinfo->src->next_input_byte += bytes;
	cinfo->src->bytes_in_buffer -= bytes;
}

//========================================================================================================
// Overriding function: Close the source of data -- do nothing
//========================================================================================================

static void OnCloseSource (j_decompress_ptr cinfo)
{
}

//========================================================================================================
// Overriding function: Error handling
//========================================================================================================

static void OnError (j_common_ptr info)
{
	jpeg_destroy(info);
	CustomData* data = (CustomData*)info->client_data;
	data->mError = true;
	longjmp(data->mJump, 1);
}

//========================================================================================================
// Overriding function: Display message -- do nothing
//========================================================================================================

static void OnMsg (jpeg_common_struct* info)
{
}

//========================================================================================================
// Jpeg format codec (.jpg)
//========================================================================================================

R5_IMAGE_CODEC(JPG)
{
	// Ensure that the header is what's expected
	bool result = false;
	uint header = *(const uint*)buff;

	if (header == 0xE0FFD8FF)
	{
		CustomData data;
		data.mError = false;

		// Set the jump point for OnError() callback above
		setjmp( data.mJump );

		if (data.mError == false)
		{
			// Set up default error handling
			jpeg_error_mgr jpgError;
			jpeg_std_error(&jpgError);

			// Set up custom error handling
			jpgError.error_exit		= OnError;
			jpgError.output_message	= OnMsg;

			// JPEG source manager
			jpeg_source_mgr source;
			source.init_source		 = OnInitSource;
			source.fill_input_buffer = OnFillBuffer;
			source.skip_input_data	 = OnSkipData;
			source.resync_to_restart = jpeg_resync_to_restart;
			source.termSource		 = OnCloseSource;
			source.next_input_byte	 = buff;
			source.bytes_in_buffer	 = size;

			// Init the main JPEG info struct
			jpeg_decompress_struct jpgInfo;
			jpeg_create_decompress( &jpgInfo );

			// Set the custom data as well as two other parameters
			jpgInfo.client_data			= &data;
			jpgInfo.src					= &source;
			jpgInfo.err					= &jpgError;
			jpgInfo.do_block_smoothing  = 1;
			jpgInfo.do_fancy_upsampling = 1;

			// Read the JPEG's header
			jpeg_read_header(&jpgInfo, 1);

			// Decompress the file
			jpeg_start_decompress(&jpgInfo);

			uint rowLength = jpgInfo.image_width * jpgInfo.numComponents;

			if		(jpgInfo.numComponents == 3)	out.mFormat = ITexture::Format::RGB;
			else if (jpgInfo.numComponents == 2)	out.mFormat = ITexture::Format::Luminance;
			else if (jpgInfo.numComponents == 1)	out.mFormat = ITexture::Format::Alpha;
			else									out.mFormat = ITexture::Format::Invalid;

			out.mWidth  = jpgInfo.image_width;
			out.mHeight = jpgInfo.image_height;
			out.mDepth  = 1;

			// Reserve the memory buffer
			byte* full = out.mBytes.Resize(rowLength * out.mHeight);

			// Flip the image right away by starting at the top and moving down
			byte* line = full + rowLength * (out.mHeight - 1);

			// Keep going through the lines
			while (jpgInfo.output_scanline < jpgInfo.output_height)
			{
				// Read another line in the JPEG file into the buffer
				if (jpeg_read_scanlines(&jpgInfo, &line, 1) == 0) break;
				line -= rowLength;
			}

			// Done decompressing the file
			jpeg_destroy_decompress(&jpgInfo);
		}

		// Cleanup
		if (data.mError) out.Release();
		else result = true;
	}
	return result;
}

}; // namespace Codec
}; // namespace R5