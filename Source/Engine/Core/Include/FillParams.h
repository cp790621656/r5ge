#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Needed parameters passed from one object to the next during the 'fill geometry' stage
//============================================================================================================

struct FillParams
{
	DrawQueue&		mDrawQueue;		// Draw queue
	const Frustum&	mFrustum;		// Frustum used to cull the scene
	Vector3f		mCamPos;		// Current camera position, used to sort objects
	Vector3f		mCamDir;		// Current camera direction
	bool			mCamChanged;	// Whether the camera has changed since last Fill() call

	FillParams (DrawQueue& q, const Frustum& f) : mDrawQueue(q), mFrustum(f) {}

	inline float GetDist(const Vector3f& pos) const { return (mCamPos - pos).Dot(); }
};