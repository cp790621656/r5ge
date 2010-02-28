#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Struct used for raycasts
//============================================================================================================

struct RaycastHit
{
	Object*		mObject;		// Pointer to the object that will be rendered
	float		mSqrCamDist;	// Squared distance to the camera, used to sort objects

	// Comparison operator for sorting
	bool operator < (const RaycastHit& obj) const { return (mSqrCamDist < obj.mSqrCamDist); }
};