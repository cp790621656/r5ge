#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Code-shortening macros used below
// Author: Michael Lyashenko
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