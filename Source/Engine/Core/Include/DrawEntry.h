#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Draw entry struct is used to sort drawable objects
//============================================================================================================

struct DrawEntry
{
	Object*		mObject;		// Pointer to the object that will be rendered
	const void*	mGroup;			// Allow grouping of similar objects in order to avoid state switches
	float		mDistSquared;	// Squared distance to the camera, used to sort objects

	// Comparison operator for sorting -- groups objects by layer, pointer, and lastly -- distance
	bool operator < (const DrawEntry& obj) const
	{
		if (mGroup < obj.mGroup) return true;
		if (mGroup > obj.mGroup) return false;
		return (mDistSquared < obj.mDistSquared);
	}
};