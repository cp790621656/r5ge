#include "../Include/_All.h"

#ifdef R5_SUPPORTS_ZLIB
  #include "../Include/zlib.h"

  #ifdef _WINDOWS
    #pragma comment(lib, "zlib.lib")
  #endif
#endif

#include "../LZMA/LzFind.c"
#include "../LZMA/LzmaEnc.c"
#include "../LZMA/LzmaDec.c"

using namespace R5;

//============================================================================================================
// 64 Kb will be used as the initial buffer size. Note that reusing the memory buffer will effectively
// save you memory allocations and deallocations, and is therefore highly recommended.
//============================================================================================================

#define BUFFER_SIZE 65536

//============================================================================================================

static void* AllocForLzma (void *p, size_t size)
{
	// Sanity check: 256 MB limit
	const uint max = (1 << 28);
#ifdef _DEBUG
	ASSERT(size < max, "Requested over 256 MB of memory -- intentional?");
#endif
	return (size < max) ? new byte[size] : 0;
}

static void FreeForLzma (void *p, void *address) { delete [] ((byte*)address); }
static ISzAlloc SzAllocForLzma = { &AllocForLzma, &FreeForLzma };

//============================================================================================================

typedef struct
{
  ISeqInStream SeqInStream;
  const byte* buffer;
  uint size;
  uint offset;
} VectorInStream;

//============================================================================================================

typedef struct
{
  ISeqOutStream SeqOutStream;
  Memory& mem;
} VectorOutStream;

//============================================================================================================
// Stream reading function used by LZMA
//============================================================================================================

int ReadStream (void* p, void* buf, uint* size)
{
	VectorInStream* stream = (VectorInStream*)p;

	uint remaining = stream->size - stream->offset;
	if (remaining < *size) *size = remaining;

	if (*size)
	{
		memcpy(buf, stream->buffer + stream->offset, *size);
		stream->offset += *size;
	}
	return SZ_OK;
}

//============================================================================================================
// Stream writing function used by LZMA
//============================================================================================================

uint WriteToMemory (void* p, const void* buf, uint size)
{
	VectorOutStream* ctx = (VectorOutStream*)p;
	if (size) ctx->mem.Append(buf, size);
	return size;
}

//============================================================================================================
// Compresses the specified chunk of data using LZMA
//============================================================================================================

bool R5::CompressLZMA (const byte* buffer, uint length, Memory& memOut)
{
	CLzmaEncHandle enc = LzmaEnc_Create(&SzAllocForLzma);

	if (enc)
	{
		CLzmaEncProps props;
		LzmaEncProps_Init(&props);
		props.writeEndMark = 1;

		if (SZ_OK == LzmaEnc_SetProps(enc, &props))
		{
			byte propData[LZMA_PROPS_SIZE];
			unsigned propsSize = LZMA_PROPS_SIZE;

			// Write the header
			if (SZ_OK == LzmaEnc_WriteProperties(enc, propData, &propsSize) &&
				propsSize == LZMA_PROPS_SIZE)
			{
				VectorInStream inStream = { &ReadStream, buffer, length, 0 };
				VectorOutStream outStream = { &WriteToMemory, memOut };

				// Append the header
				memOut.Append(propData, propsSize);

				// Encode the entire memory stream
				bool retVal = (SZ_OK == LzmaEnc_Encode(enc,
					(ISeqOutStream*)&outStream,
					(ISeqInStream*)&inStream,
					0, &SzAllocForLzma, &SzAllocForLzma));

				ASSERT(retVal, "Failed to compress the stream");
				return retVal;
			}
		}

		// Free memory
		LzmaEnc_Destroy(enc, &SzAllocForLzma, &SzAllocForLzma);
	}
	return false;
}

//============================================================================================================
// Decompresses the specified chunk of data using LZMA
//============================================================================================================

bool R5::DecompressLZMA (const byte* buffer, uint length, Memory& memOut)
{
	bool retVal = false;
	CLzmaDec dec;  
	LzmaDec_Construct(&dec);

	if (SZ_OK == LzmaDec_Allocate(&dec, buffer, LZMA_PROPS_SIZE, &SzAllocForLzma))
	{
		LzmaDec_Init(&dec);

		// Skip the bytes used by the header
		unsigned outOffset = 0, inOffset = LZMA_PROPS_SIZE;

		// Start with a decently sized chunk of memory
		memOut.Resize(BUFFER_SIZE);

		ELzmaStatus status;

		while (inOffset < length)
		{
			unsigned inSize  = length - inOffset;
			unsigned outSize = memOut.GetSize() - outOffset;

			// Ensure we always have enough space to work with
			if (outSize < BUFFER_SIZE)
			{
				memOut.Resize(memOut.GetSize() + BUFFER_SIZE);
				outSize = BUFFER_SIZE;
			}

			// Decode the buffer
			uint result = LzmaDec_DecodeToBuf(&dec, memOut.GetBuffer() + outOffset, &outSize,
				buffer + inOffset, &inSize, LZMA_FINISH_ANY, &status);

			if (result == SZ_OK)
			{
				inOffset += inSize;
				outOffset += outSize;

				// Keep going until we finish the decoding process
				if (status != LZMA_STATUS_FINISHED_WITH_MARK) continue;
				retVal = true;
			}
			break;
		}
		memOut.Resize(outOffset);
	}
	LzmaDec_Free(&dec, &SzAllocForLzma);
	return retVal;
}

#ifdef R5_SUPPORTS_ZLIB

//============================================================================================================
// Compresses the specified chunk of data using ZLIB
//============================================================================================================

bool R5::CompressZLIB (const byte* buffer, uint length, Memory& mem)
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
// Uncompresses the specified chunk of data using ZLIB
//============================================================================================================

bool R5::DecompressZLIB (const byte* buffer, uint length, Memory& mem)
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
#endif