#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Sets the slider's value directly
//============================================================================================================

void UIAnimatedSlider::SetValue (float val)
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

bool UIAnimatedSlider::OnUpdate (bool dimensionsChanged)
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

bool UIAnimatedSlider::CustomSerializeFrom(const TreeNode& root)
{
	if (root.mTag == "Animation Time")
	{
		root.mValue >> mAnimTime;
		return true;
	}
	return UISlider::CustomSerializeFrom(root);
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIAnimatedSlider::CustomSerializeTo(TreeNode& root) const
{
	UISlider::CustomSerializeTo(root);
	root.AddChild("Animation Time", mAnimTime);
}