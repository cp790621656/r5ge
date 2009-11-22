#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Sets the slider's value directly
//============================================================================================================

void AnimatedSlider::SetValue (float val)
{
	val = Float::Clamp(val, 0.0f, 1.0f);

	if ( Float::IsNotEqual(mVal, val) )
	{
		mStartVal  = mVal;
		mTargetVal = val;
		mStartTime = mRoot->GetCurrentTime();
	}
}

//============================================================================================================
// Updates the slider's value
//============================================================================================================

bool AnimatedSlider::OnUpdate (bool dimensionsChanged)
{
	if ( Float::IsNotEqual(mVal, mTargetVal) )
	{
		float time = mRoot->GetCurrentTime();
		float factor = (mAnimTime > 0.0f) ? Float::Clamp((time - mStartTime) / mAnimTime, 0.0f, 1.0f) : 1.0f;
		mVal = mStartVal * (1.0f - factor) + mTargetVal * factor;

		// Trigger the callback
		if (mOnValueChange) mOnValueChange(this);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool AnimatedSlider::CustomSerializeFrom(const TreeNode& root)
{
	if (root.mTag == "Animation Time")
	{
		root.mValue >> mAnimTime;
		return true;
	}
	return Slider::CustomSerializeFrom(root);
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void AnimatedSlider::CustomSerializeTo(TreeNode& root) const
{
	Slider::CustomSerializeTo(root);
	root.AddChild("Animation Time", mAnimTime);
}