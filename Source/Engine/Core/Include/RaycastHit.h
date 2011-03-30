#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Struct used for raycasts
//============================================================================================================

struct RaycastHit
{
	Object*	mObject;
	float	mDistanceToCameraSquared;
	//float	mDistanceToCenter;

	// Comparison operator for sorting
	bool operator < (const RaycastHit& obj) const { return (mDistanceToCameraSquared < obj.mDistanceToCameraSquared); }
};