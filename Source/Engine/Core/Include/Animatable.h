#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Animatable placeable -- placeable node with has splines it can follow
// Author: Michael Lyashenko
//============================================================================================================

class Animatable : public Object
{
protected:

	SplineV		mPosSpline;		// Position spline with time keys in 0 to 1 range
	SplineQ		mRotSpline;		// Rotation spline
	SplineV		mScaleSpline;	// Scale spline

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Animatable", Animatable, Object, Object);

	// Whether the node is considered to be animated
	bool IsAnimated() const	{ return (mPosSpline.GetSize() > 1 ||
									  mRotSpline.GetSize() > 1 ||
									  mScaleSpline.GetSize() > 1); }

	// Clears all animation keys
	void ClearKeys() { mPosSpline.Clear();
					   mRotSpline.Clear();
					   mScaleSpline.Clear(); }

	// Keyframe control -- keys should be added in 0 to 1 time range
	void AddPosKey   (float time, const Vector3f& pos)		{ mPosSpline.AddKey(time, pos);	 }
	void AddRotKey   (float time, const Quaternion& rot)	{ mRotSpline.AddKey(time, rot);	 }
	void AddScaleKey (float time, const Vector3f& scale)	{ mScaleSpline.AddKey(time, scale); }

	// Updates the relative positions by sampling the splines
	void SetTime (float val);
};