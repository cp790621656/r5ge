#include "../Include/_All.h"
#include "../Include/_Codecs.h"

namespace R5
{
namespace Codec
{

//========================================================================================================
// Converts an RGBE value into RGB32F
//========================================================================================================

inline void RGBEToFloat (const byte* buffer, uint width, float* out)
{
	int exponent = buffer[3*width];

	if (exponent != 0)
	{
		float f = ldexp(1.0f, exponent - (128+8));
		out[0] = f * buffer[0];
		out[1] = f * buffer[width];
		out[2] = f * buffer[width*2];
	}
	else
	{
		out[0] = 0.0f;
		out[1] = 0.0f;
		out[2] = 0.0f;
	}
}

//========================================================================================================
// Reads RGBE pixels and converts them to RGB32F
//========================================================================================================

inline void Read (const byte* buffer, uint size, float *out, uint width = 1)
{
	for (uint i = 0; i < size; i+=4, out+=3)
		RGBEToFloat(buffer+i, width, out);
}

//========================================================================================================
// Read the compressed Radiance file format
//========================================================================================================

bool ReadCompressed (const byte*	buffer,
					 uint			size,
					 float*					out,
					 uint			width,
					 uint			height)
{
	uint index = 0;

	// No encoding -- treat it as an uncompressed radiance file
	if ( width < 8 || width > 0x7FFF || buffer[index] != 2 || buffer[index+1] != 2 || buffer[index+2] & 0x80 )
	{
		WARNING("Loading of an uncompressed Radiance file format is untested, and does not currently flip the image");
		Read(buffer + index, width * height, out);
		return true;
	}

	// Every line will need to be decoded into a buffer
	byte* line (0);

	// Run through all lines
	for (uint h = 0; h < height; ++h )
	{
		// Safety check
		if (index + 4 >= size) break;

		// Scanline must begin with '22' and must have its width match the image's width
		if ( buffer[index] != 2 || buffer[index+1] != 2 || (((int)buffer[index+2]) << 8 | buffer[index+3]) != width )
		{
			ASSERT(false, "Compressed Radiance file format read error");
			if (line)	delete [] line;
			if (buffer) delete [] buffer;
			return false;
		}

		// Advance the index past the line header
		index += 4;

		// A temporary line must be created in order to hold the decoded data
		if (line == 0) line = new byte[width*4];
		byte* ptr = line;

		// Decode each of the four channels into the line
		for (uint i = 0; i < 4; ++i)
		{
			byte* ptr_end = &line[(i+1)*width];

			// Keep going until the line reaches the end
			while (ptr < ptr_end)
			{
				byte code  = buffer[index++];
				byte value = buffer[index++];

				// Repeating value
				if (code > 128)
				{
					uint count = code - 128;

					// Safety check
					if ( (count == 0) || (count > (uint)(ptr_end - ptr)) )
					{
						delete [] line;
						return false;
					}

					// Copy the bytes
					while (count-- > 0) *ptr++ = value;
				}
				else // Unique values follow
				{
					uint count = code;

					if ( (count == 0) || (count > (uint)(ptr_end - ptr)) )
					{
						delete [] line;
						return false;
					}

					*ptr++ = value;

					if (--count > 0)
					{
						for (uint b = 0; b < count; ++b)
							*ptr++ = buffer[index++];
					}
				}
			}
		}

		// The image should be flipped so it's bottom-left based
		float* dataPtr = out + (height - h - 1) * 3 * width;

		// Convert the line data from bytes to floats
		for (uint i = 0; i < width; ++i, dataPtr+=3)
			RGBEToFloat(line+i, width, dataPtr);
	}
	if (line) delete [] line;
	return true;
}

//========================================================================================================
// Radiance format codec (.hdr)
//========================================================================================================

R5_READ_IMAGE_CODEC(HDR)
{
	// Ensure that the header is what's expected
	uint header = *(const uint*)buff;

	if (header == 0x41523F23)
	{
		// Ensure that the outgoing data is cleared
		out.Release();

		// Skip past the header
		buff += 4;
		size -= 4;

		// Width and height of the image are retrieved before the actual image's data
		uint width (0), height (0);

		// Skip past the comments and "FORMAT" sections
		for (; size > 16; --size, ++buff)
		{
			// Find the dimensions of the image buff format: -Y 512 +X 512
			if (buff[0] == '-' && buff[1] == 'Y' && sscanf((const char*)buff, "-Y %u +X %u", &height, &width) == 2)
			{
				out.mWidth  = width;
				out.mHeight = height;
				out.mDepth  = 1;
				out.mFormat = Image::Format::RGB32F;

				// Skip past the minimum length of the string above
				buff += 10;
				size -= 10;

				// Skip to the new line character
				while (size > 1 && buff[0] != '\n') { ++buff; --size; }

				// Skip past the new line character
				++buff;
				--size;
				break;
			}
		}

		if (size > 1)
		{
			// Allocate a new buffer -- 3 floats per pixel (RGB)
			float* ptr = (float*)out.mBytes.Resize(width * height * 3 * sizeof(float));

			// Read and decode the Radiance file
			if (ReadCompressed(buff, size, ptr, width, height)) return true;
		}
	}
	return false;
}

}; // namespace Codec
}; // namespace R5