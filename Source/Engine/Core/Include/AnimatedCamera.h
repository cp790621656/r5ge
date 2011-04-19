#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Extended camera class supporting spline-based animation
// Author: Michael Lyashenko
//============================================================================================================

class AnimatedCamera : public Camera
{
protected:

	SplineV	mV;
	SplineQ	mQ;
	SplineF mF;

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Animated Camera", AnimatedCamera, Camera, Object);

	// Clear the animation splines
	void Clear() { mV.Clear(); mQ.Clear(); mF.Clear(); }

	// Animate to the specified relative orientation over the specified amount of time
	void AnimateTo (const Vector3f& pos, const Quaternion& rot, float fov, float delay);

protected:

	// Modify the relative coordinates based on sampled splines
	virtual void OnPreUpdate();
};