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
		(buff[4] == 'T' || buff[4] == 'X'))
	{
		bool interleaved = (buff[4] == 'T');

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
				uint bpp = ITexture::GetBitsPerPixel(format) >> 3;
				uint length = bpp * width * height;

				// We should have only the color information left
				if (length > 0 && size == length)
				{
					byte* outBuff = out.mBytes.Resize(length);

					if (interleaved || bpp == 1)
					{
						return Memory::Extract(buff, size, outBuff, length);
					}
					else
					{
						// New format uses non-interleaved colors as they compress better
						uint pixels = width * height;

						for (uint i = 0, j = 0; i < size; ++j)
						{
							for (uint c = 0; c < bpp; ++c)
							{
								outBuff[i++] = buff[j + pixels * c];
							}
						}

						buff += length;
						size -= length;
						return true;
					}
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
	uint channels = ITexture::GetBitsPerPixel(format) >> 3;

	if (channels < 2)
	{
		uint size = width * height * channels;
		out.Append("//R5T", 5);
		out.Append((ushort)width);
		out.Append((ushort)height);
		out.Append((ushort)format);
		return Compress(buffer, size, out);
	}

	uint size = width * height * channels;
	Memory r5t, r5x;

	// R5X approach: non-interleaved colors
	r5t.Reserve(size + 11);
	r5x.Reserve(size + 11);
	r5x.Append("//R5X", 5);
	r5x.Append((ushort)width);
	r5x.Append((ushort)height);
	r5x.Append((ushort)format);
	
	//uint header = r5t.GetSize();
	byte* temp = r5t.Resize(size);

	// We need to write all reds, followed by all greens, etc
	for (uint channel = 0, offset = 0; channel < channels; ++channel)
	{
		for (uint i = channel; i < size; i += channels)
		{
			temp[offset++] = buffer[i];
		}
	}

	// Compress the result
	if (!Compress(temp, size, r5x)) return false;

	// R5T approach: interleaved colors
	r5t.Clear();
	r5t.Append("//R5T", 5);
	r5t.Append((ushort)width);
	r5t.Append((ushort)height);
	r5t.Append((ushort)format);

	// Non-interleaved approach is simple: just compress the data
	if (!Compress(buffer, size, r5t)) return false;

	// Choose the smallest of the two buffers
	if (r5t.GetSize() < r5x.GetSize())
	{
		out.Append(r5t.GetBuffer(), r5t.GetSize());
	}
	else
	{
		out.Append(r5x.GetBuffer(), r5x.GetSize());
	}
	return true;
}

}; // namespace Codec
}; // namespace 