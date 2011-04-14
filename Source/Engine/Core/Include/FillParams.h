#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Needed parameters passed from one object to the next during the 'fill geometry' stage
// Author: Michael Lyashenko
//============================================================================================================

struct FillParams
{
	DrawQueue&		mDrawQueue;		// Draw queue
	const Frustum&	mFrustum;		// Frustum used to cull the scene
	Vector3f		mCamPos;		// Current camera position, used to sort objects
	Vector3f		mCamDir;		// Current camera direction
	const Object*	mEye;			// Camera associated with the current fill process

	FillParams (DrawQueue& q, const Frustum& f) : mDrawQueue(q), mFrustum(f), mEye(0) {}

	inline float GetDist(const Vector3f& pos) const { return (mCamPos - pos).Dot(); }
};