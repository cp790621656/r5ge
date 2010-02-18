#include "../Include/_All.h"
#include "../Include/_Codecs.h"

namespace R5
{
namespace Codec
{
//========================================================================================================
// R5 compressed texture file format
//========================================================================================================

R5_READ_IMAGE_CODEC(R5T)
{
	if (size > 5 &&
		buff[0] == '/' &&
		buff[1] == '/' &&
		buff[2] == 'R' &&
		buff[3] == '5' &&
		buff[4] == 'T')
	{
		buff += 5;
		size -= 5;

		ushort width, height, format;

		// Extract the image width, height, and format information
		if (Memory::Extract(buff, size, width) &&
			Memory::Extract(buff, size, height) &&
			Memory::Extract(buff, size, format))
		{
			Memory mem;

			// Decompress the texture
			if (Decompress(buff, size, mem))
			{
				buff = mem.GetBuffer();
				size = mem.GetSize();

				out.mWidth  = width;
				out.mHeight = height;
				out.mDepth  = 1;
				out.mFormat = format;

				// Reserve the memory buffer for the image
				uint bpp = ITexture::GetBitsPerPixel(format) / 8;
				uint length = bpp * width * height;

				// We should have only the color information left
				if (length > 0 && size == length)
				{
					// Extract the image data
					return Memory::Extract(buff, size, out.mBytes.Resize(length), length);
				}
			}
		}
	}
	return false;
}

//========================================================================================================
// Save the specified image data in the R5 Texture file format
//========================================================================================================

R5_WRITE_IMAGE_CODEC(R5T)
{
	uint size = width * height * (ITexture::GetBitsPerPixel(format) / 8);
	out.Append("//R5T", 5);
	out.Append((ushort)width);
	out.Append((ushort)height);
	out.Append((ushort)format);
	return Compress(buffer, size, out);
}

}; // namespace Codec
}; // namespace 