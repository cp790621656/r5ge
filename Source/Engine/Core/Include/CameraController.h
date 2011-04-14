#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Camera Controller provides a way to blend together different cameras
// Author: Michael Lyashenko
//============================================================================================================

class CameraController
{
private:

	struct Entry
	{
		Camera*		mCamera;
		float		mAlpha;
		bool		mIsFading;
		bool		mRemove;
	};

	Vector3f		mPos;
	Quaternion		mRot;
	Vector3f		mRan;
	Array<Entry>	mCameras;
	float			mFadeTime;

public:

	CameraController() : mRan(0.3f, 100.0f, 90.0f), mFadeTime(0.25f) {}

	// Release all managed cameras
	void Release() { mCameras.Lock(); mCameras.Release(); mCameras.Unlock(); }

	// Calculated values
	const Vector3f&		GetPosition()	const	{ return mPos; }
	const Quaternion&	GetRotation()	const	{ return mRot; }
	const Vector3f&		GetRange()		const	{ return mRan; }

	// Note that these functions are only meant to set the starting values
	void SetPosition (const Vector3f& pos)		{ mPos = pos; }
	void SetRotation (const Quaternion& rot)	{ mRot = rot; }
	void SetRange	 (const Vector3f& ran)		{ mRan = ran; }

	// Controls how many seconds it takes for cameras to fade between one another
	float GetFadeTime() const		{ return mFadeTime; }
	void  SetFadeTime (float val)	{ mFadeTime = val; }

	// Returns the most recently added camera that's not currently fading out
	const Camera* GetActiveCamera() const;

	// Adds a new camera to the controller. The new camera will be smoothly faded in.
	void Activate (Camera* cam);

	// Deactivates an existing camera, starting the fading process. The camera will be removed at the end.
	void Deactivate (const Camera* cam);

	// Immediately removes the specified camera
	void Remove (const Camera* cam);

	// Updates the pos/rot/range, blending the cameras together
	void Update();
};