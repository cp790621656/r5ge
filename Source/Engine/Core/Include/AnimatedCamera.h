#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Extended camera class supporting spline-based animation
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