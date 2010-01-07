#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Collection of common codecs
//============================================================================================================

#define R5_IMAGE_CODEC(name) bool name (const byte*			buff,		\
										uint				size,		\
										const String&		extension,	\
										R5::Image::Buffer&	out )

namespace R5
{
	namespace Codec
	{
		R5_IMAGE_CODEC(HDR);		// Radiance (.hdr) file format
		R5_IMAGE_CODEC(PNG);		// Portable Network Graphics (.png) file format
		R5_IMAGE_CODEC(JPG);		// JPEG (.jpg) file format
	};
};