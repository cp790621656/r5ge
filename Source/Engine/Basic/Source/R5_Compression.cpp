#include "../Include/_All.h"
#include "../Include/zlib.h"
using namespace R5;

#ifdef _WINDOWS
#pragma comment(lib, "zlib.lib")
#endif

//============================================================================================================
// 64 Kb will be used as the initial buffer size. Note that reusing the memory buffer will effectively
// save you memory allocations and deallocations, and is therefore highly recommended.
//============================================================================================================

#define BUFFER_SIZE 65536

//============================================================================================================
// Compresses the specified chunk of data
//============================================================================================================

bool R5::Compress (const byte* buffer, uint length, Memory& mem)
{
	if (buffer != 0 && length > 0)
	{
		// Initialization values -- no custom memory management functions
		z_stream strm;
		strm.zalloc = 0;
		strm.zfree	= 0;
		strm.opaque = 0;

		// Init the ZLib library
		if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) == Z_OK)
		{
			// Use the full incoming memory buffer for decompression
			strm.next_in	= (Bytef*)buffer;
			strm.avail_in	= length;
			strm.avail_out	= 0;

			// Keep going while there is data to process
			while (strm.avail_out == 0)
			{
				uint start		= mem.GetSize();
				strm.next_out	= (Bytef*)mem.Expand(BUFFER_SIZE);
				strm.avail_out	= mem.GetSize() - start;

				// Compress the data
				deflate(&strm, Z_FINISH);

				// If the outgoing stream still has some available space it means we've reached the end
				if (strm.avail_out > 0)
				{
					uint final = mem.GetSize() - strm.avail_out;
					mem.Resize(final);
				}
			}
			deflateEnd(&strm);
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Uncompresses the specified chunk of data
//============================================================================================================

bool R5::Decompress (const byte* buffer, uint length, Memory& mem)
{
	if (buffer != 0 && length > 0)
	{
		// Initialization values -- no custom memory management functions
		z_stream strm;
		strm.zalloc		= 0;
		strm.zfree		= 0;
		strm.opaque		= 0;
		strm.avail_in	= 0;
		strm.next_in	= 0;

		if (inflateInit(&strm) == Z_OK)
		{
			strm.next_in	= (Bytef*)buffer;
			strm.avail_in	= length;
			strm.avail_out	= 0;

			while (strm.avail_out == 0)
			{
				uint start		= mem.GetSize();
				strm.next_out	= (Bytef*)mem.Expand(BUFFER_SIZE);
				strm.avail_out	= mem.GetSize() - start;

				// Uncompress the data
				int retVal = inflate(&strm, Z_FINISH);

				// Negative return value means an error has occurred
				if ((retVal < 0 && retVal != Z_BUF_ERROR) || retVal == Z_NEED_DICT)
				{
					inflateEnd(&strm);
					return false;
				}

				// If the outgoing stream still has some available space it means we've reached the end
				if (strm.avail_out > 0)
				{
					uint final = mem.GetSize() - strm.avail_out;
					mem.Resize(final);
				}
			}
			inflateEnd(&strm);
			return true;
		}
	}
	return false;
}