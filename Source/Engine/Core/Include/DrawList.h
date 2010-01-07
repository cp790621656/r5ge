#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// All drawable objects are separated by layers into different draw lists
//============================================================================================================

struct DrawList
{
	Array<DrawEntry> mEntries;

	// Draw all objects in the list
	uint Draw (const ITechnique* tech, bool insideOut);
};