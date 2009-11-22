#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Slider that smoothly animated to its set value
//============================================================================================================

class UIAnimatedSlider : public UISlider
{
protected:

	float	mStartVal;	// Value of the slider at the start of the animation
	float	mTargetVal;	// Target value
	float	mStartTime;	// Start time
	float	mAnimTime;	// Time to animate from start to target value in seconds

public:

	UIAnimatedSlider() : mStartVal(0.0f), mTargetVal(0.0f), mStartTime(0), mAnimTime(0.0f) {}

	float GetAnimationTime() const		{ return mAnimTime; }
	void  SetAnimationTime(float val)	{ mAnimTime = val;  }

	// Return the target value rather than the current value
	virtual float	GetValue()	 const	{ return mTargetVal; }

	// Sets the slider's value directly
	virtual void SetValue (float val);

public:

	// Area creation
	static UIArea* _CreateNew() { return new UIAnimatedSlider(); }

	// Updates the slider's value
	virtual bool OnUpdate (bool dimensionsChanged);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};