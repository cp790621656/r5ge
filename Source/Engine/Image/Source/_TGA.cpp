#include "../Include/_All.h"
#include "../Include/_Codecs.h"

namespace R5
{
namespace Codec
{
//========================================================================================================
// Targa format codec (.tga)
//========================================================================================================

R5_READ_IMAGE_CODEC(TGA)
{
	// Ensure that the header is what's expected
	uint header = *(const uint*)buff;

	if (header == 0x20000 && size > 12)
	{
		// Ensure that the outgoing data is cleared
		out.Release();

		// Skip past the header
		buff += 12;
		size -= 12;

		ushort width, height;
		byte bitrate;

		// Extract some useful information
		if (Memory::Extract(buff, size, width) &&
			Memory::Extract(buff, size, height) &&
			Memory::Extract(buff, size, bitrate))
		{
			// Skip the image descriptor byte
			buff += 1;
			size -= 1;

			// Bytes per pixel
			uint bpp = bitrate >> 3;

			// Allocate a new buffer
			uint length = bpp * width * height;

			// Ensure that we have some data to work with
			if (length >= bpp && (bpp == 3 || bpp == 4))
			{
				out.mWidth  = width;
				out.mHeight = height;
				out.mDepth  = 1;
				out.mFormat = (bpp == 3) ? ITexture::Format::RGB : ITexture::Format::RGBA;

				// Reserve the memory buffer for the image
				byte* ptr = out.mBytes.Resize(length);

				// Extract the image data
				if (Memory::Extract(buff, size, ptr, length))
				{
					while (size > 0)
					{
						--size;
						++buff;
					}

					// Convert BGRA to RGBA
					for (uint i = 0; i < length; i += bpp)
					{
						ptr[i] ^= ptr[i+2] ^= ptr[i] ^= ptr[i+2];
					}
					return true;
				}
			}
		}
	}
	return false;
}

//========================================================================================================
// Save as Targa format
//========================================================================================================

R5_WRITE_IMAGE_CODEC(TGA)
{
	uint size = width * height * (ITexture::GetBitsPerPixel(format) / 8);

	if (format == ITexture::Format::RGB)
	{
		format = 24;
	}
	else if (format == ITexture::Format::RGBA)
	{
		format = 32;
	}
	else
	{
		WARNING("TGA format export only supports RGB and RGBA formats");
		return false;
	}

	// Append the header identifying it as the TGA file
	out.Append((uint)0x20000);
	out.Append((uint)0x0);
	out.Append((uint)0x0);
	out.Append((ushort)width);
	out.Append((ushort)height);
	out.Append((byte)format);
	out.Append(format == 32 ? (byte)8 : (byte)0);

	// We want to be dealing with bytes now, not bits
	format /= 8;

	// Append the image data
	byte* outBuffer = (byte*)out.Append(buffer, size);

	// Flip RGBA to BGRA as that's what TGA expects
	for (uint i = 0; i < size; i += format)
	{
		outBuffer[i] ^= outBuffer[i+2] ^= outBuffer[i] ^= outBuffer[i+2];
	}

	// Append the footer
	out.Append("TRUEVISION-XFILE.\0", 18);

	// Finally save the memory to the specified file
	return true;
}

}; // namespace Codec
}; // namespace 