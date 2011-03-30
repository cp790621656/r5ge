#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Compression functionality
//============================================================================================================

#define R5_SUPPORTS_ZLIB

// Compresses the specified chunk of data using LZMA
bool CompressLZMA (const byte* buffer, uint length, Memory& memOut);

// Decompresses the specified chunk of data using LZMA
bool DecompressLZMA (const byte* buffer, uint length, Memory& memOut);

// ZLIB compression requires linking of an external library, so it's optional
#ifdef R5_SUPPORTS_ZLIB

// Compresses the specified chunk of data using ZLIB
bool CompressZLIB (const byte* buffer, uint length, Memory& memOut);

// Decompresses the specified chunk of data using ZLIB
bool DecompressZLIB (const byte* buffer, uint length, Memory& memOut);

#endif

// Compresses the specified chunk of data using the best compression algorithm
inline bool Compress (const byte* buffer, uint length, Memory& memOut) { return CompressLZMA(buffer, length, memOut); }

// Decompresses the specified chunk of data using the best compression algorithm
inline bool Decompress (const byte* buffer, uint length, Memory& memOut)
{
#ifdef R5_SUPPORTS_ZLIB
	return DecompressZLIB(buffer, length, memOut) || DecompressLZMA(buffer, length, memOut);
#else
	return DecompressLZMA(buffer, length, memOut);
#endif
}

// Convenience functionality
inline bool Compress	(const Memory& memIn, Memory& memOut) { return CompressLZMA(memIn.GetBuffer(), memIn.GetSize(), memOut); }
inline bool Decompress	(const Memory& memIn, Memory& memOut) { return DecompressLZMA(memIn.GetBuffer(), memIn.GetSize(), memOut); }