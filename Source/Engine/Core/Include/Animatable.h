#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Animatable placeable -- placeable node with has splines it can follow
//============================================================================================================

class Animatable : public Object
{
protected:

	SplineV		mSplineV;	// Position spline with time keys in 0 to 1 range
	SplineQ		mSplineQ;	// Rotation spline
	SplineF		mSplineF;	// Scale spline

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Animatable", Animatable, Object, Object);

	// Whether the node is considered to be animated
	bool IsAnimated() const	{ return (mSplineV.GetSize() > 1 ||
									  mSplineQ.GetSize() > 1 ||
									  mSplineF.GetSize() > 1); }

	// Clears all animation keys
	void ClearKeys() { mSplineV.Clear();
					   mSplineQ.Clear();
					   mSplineF.Clear(); }

	// Keyframe control -- keys should be added in 0 to 1 time range
	void AddKey (float time, const Vector3f& pos)		{ mSplineV.AddKey(time, pos);	 }
	void AddKey (float time, const Quaternion& rot)		{ mSplineQ.AddKey(time, rot);	 }
	void AddKey (float time, float scale)				{ mSplineF.AddKey(time, scale); }

	// Updates the relative positions by sampling the splines
	void SetTime (float val);
};