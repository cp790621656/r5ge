#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Code-shortening macros used below
//============================================================================================================

#define R5_READ_IMAGE_CODEC(name) bool Read##name \
	(const byte*		buff,		\
	 uint				size,		\
	 const String&		extension,	\
	 R5::Image::Buffer&	out )

#define R5_WRITE_IMAGE_CODEC(name) bool Write##name \
	(Memory&		out,	\
	 const byte*	buffer,	\
	 uint			width,	\
	 uint			height,	\
	 uint			format)

//============================================================================================================
// Collection of common codecs
//============================================================================================================

namespace R5
{
	namespace Codec
	{
		R5_READ_IMAGE_CODEC(HDR);		// Radiance (.hdr) file format
		R5_READ_IMAGE_CODEC(PNG);		// Portable Network Graphics (.png) file format
		R5_READ_IMAGE_CODEC(JPG);		// JPEG (.jpg) file format
		R5_READ_IMAGE_CODEC(TGA);		// Targa (.tga) file format
		R5_READ_IMAGE_CODEC(R5T);		// R5 compressed texture file format (.r5t)

		R5_WRITE_IMAGE_CODEC(TGA);
		R5_WRITE_IMAGE_CODEC(R5T);
	};
};