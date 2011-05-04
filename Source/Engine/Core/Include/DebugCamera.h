#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Free movement camera, useful for debugging
// Author: Michael Lyashenko
//============================================================================================================

class DebugCamera : public Camera
{
protected:

	Vector3f	mDolly;				// Distance from the origin: X = Min, Y = Current, Z = Max
	Vector3f	mFriction;			// Friction multiplier applied every millisecond (0.97 = 3%)
	Vector3f	mPosMovement;		// Current positional movement
	Vector2f	mRotMovement;		// Current rotational movement
	Vector3f	mSavedPos;			// Saved relative position, restored in post-update
	float		mDollyMovement;		// Current dolly movement
	bool		mHasMovement;		// Whether there is potential movement in progress
	SplineV		mPosSpline;
	SplineQ		mRotSpline;
	SplineF		mDollySpline;

public:

	DebugCamera();

	// Object creation
	R5_DECLARE_INHERITED_CLASS(DebugCamera, Camera, Object);

	// Dolly vector contains how close to the camera it can get (X), current value (Y), and maximum distance (Z)
	const Vector3f& GetDolly() const	{ return mDolly; }
	void SetDolly (const Vector3f& val) { mDolly = val; }
	void SetDolly (float val)			{ mDolly.y = Float::Clamp(val, mDolly.x, mDolly.z); }

	// Stops the camera in its tracks
	void Stop (bool animation = true);

	// Animates the camera to the specified destination over the specified amount of seconds
	void AnimateTo (const Vector3f& pos, const Quaternion& rot, float delay) { AnimateTo(pos, rot, mDolly.y, delay); }
	void AnimateTo (const Vector3f& pos, const Quaternion& rot, float dolly, float delay);

	// Respond to mouse movement
	uint MouseMove(const Vector2i& pos, const Vector2i& delta);
	uint Scroll	(const Vector2i& pos, float delta);

private:

	// Updates the position and rotation based on the movement and returns the dolly-offset position
	Vector3f _UpdateOffsetPosition();

protected:

	// Register the event listener callbacks
	virtual void OnInit();
	virtual void OnDestroy();

	// Update functions differ as they need to use the dolly-offset relative position
	virtual void OnPreUpdate();
	virtual void OnPostUpdate();

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};