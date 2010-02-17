#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// ZLIB-based compression functionality
//============================================================================================================

// Compresses the specified chunk of data
bool Compress (const byte* buffer, uint length, Memory& memOut);

// Decompresses the specified chunk of data
bool Decompress (const byte* buffer, uint length, Memory& memOut);

// Convenience functionality
inline bool Compress	(const Memory& memIn, Memory& memOut) { return Compress(memIn.GetBuffer(), memIn.GetSize(), memOut); }
inline bool Decompress	(const Memory& memIn, Memory& memOut) { return Decompress(memIn.GetBuffer(), memIn.GetSize(), memOut); }