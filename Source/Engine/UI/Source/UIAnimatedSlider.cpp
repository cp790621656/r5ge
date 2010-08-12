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
		mStartTime = mUI->GetCurrentTime();
	}
}

//============================================================================================================
// Updates the slider's value
//============================================================================================================

bool UIAnimatedSlider::OnUpdate (bool dimensionsChanged)
{
	if ( Float::IsNotEqual(mVal, mTargetVal) )
	{
		if (GetAlpha() == 0.0f)
		{
			mVal = mTargetVal;
		}
		else
		{
			float time = mUI->GetCurrentTime();
			float factor = (mAnimTime > 0.0f) ? Float::Clamp((time - mStartTime) / mAnimTime, 0.0f, 1.0f) : 1.0f;
			mVal = mStartVal * (1.0f - factor) + mTargetVal * factor;
		}
		// Trigger the callback
		OnValueChange();
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIAnimatedSlider::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Animation Time")
	{
		node.mValue >> mAnimTime;
		return true;
	}
	return UISlider::OnSerializeFrom(node);
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIAnimatedSlider::OnSerializeTo (TreeNode& node) const
{
	UISlider::OnSerializeTo(node);
	node.AddChild("Animation Time", mAnimTime);
}