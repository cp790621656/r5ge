#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Struct used for raycasts
// Author: Michael Lyashenko
//============================================================================================================

struct RaycastHit
{
	Object*	mObject;
	float	mDistanceToCameraSquared;
	//float	mDistanceToCenter;

	// Comparison operator for sorting
	bool operator < (const RaycastHit& obj) const { return (mDistanceToCameraSquared < obj.mDistanceToCameraSquared); }
};