#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIAnimatedFrame::UIAnimatedFrame() :
	mStartAlpha		(0.0f),
	mCurrentAlpha	(0.0f),
	mStartTime		(0.0f),
	mAnimTime		(0.15f) {}

//============================================================================================================
// Changes the alpha
//============================================================================================================

void UIAnimatedFrame::SetAlpha (float val, float animTime)
{
	if (mRegion.GetAlpha() != val)
	{
		mIgnoreEvents	= true;
		mAnimTime		= animTime;
		mStartAlpha		= mRegion.GetParentAlpha() * mCurrentAlpha;
		mStartTime		= mUI->GetCurrentTime();
		mRegion.SetAlpha(val);
	}
}

//============================================================================================================
// Called before OnUpdate(); can be used to override the widget's alpha or dimensions
//============================================================================================================

bool UIAnimatedFrame::OnPreUpdate (bool dimensionsChanged)
{
	// Target alpha is always the region's relative alpha
	float targetAlpha = mRegion.GetAlpha();

	// Current alpha hasn't reached the target yet -- interpolate the new value based on time
	if ( Float::IsNotEqual(mCurrentAlpha, targetAlpha) )
	{
		// Update the current alpha
		float time = mUI->GetCurrentTime();
		float factor = (mAnimTime > 0.0f) ? Float::Clamp((time - mStartTime) / mAnimTime, 0.0f, 1.0f) : 1.0f;
		mCurrentAlpha = mStartAlpha * (1.0f - factor) + targetAlpha * factor;

		// Override the region's current alpha with the calculated value
		mRegion.OverrideAlpha( mRegion.GetParentAlpha() * mCurrentAlpha );

		// Enable events if the animation is finished
		if (Float::IsEqual(factor, 1.0f)) mIgnoreEvents = false;
		return true;
	}
	return false;
}