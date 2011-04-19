#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Slider that smoothly animated to its set value
// Author: Michael Lyashenko
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
	static UIWidget* _CreateNew() { return new UIAnimatedSlider(); }

	// Updates the slider's value
	virtual bool OnUpdate (bool dimensionsChanged);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};