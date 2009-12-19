#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIAnimatedButton::UIAnimatedButton() : mAnimTime(0.15f)
{
	memset(mCurrentAlpha,	0, sizeof(float)*2);
	memset(mTargetAlpha,	0, sizeof(float)*2);
	memset(mStartTime,		0, sizeof(float)*2);
}

//============================================================================================================
// Changes the button's state
//============================================================================================================

bool UIAnimatedButton::SetState (uint state, bool val)
{
	if ( UIButton::SetState(state, val) )
	{
		float current = mRoot->GetCurrentTime();

		// Clear all targets
		for (uint i = 0; i < 2; ++i)
		{
			mStartAlpha [i] = mCurrentAlpha[i];
			mTargetAlpha[i] = 0.0f;
			mStartTime  [i] = current;
		}

		// Set the appropriate target as necessary
		if (mState & State::Highlighted)	mTargetAlpha[0] = 1.0f;
		if (mState & State::Pressed)		mTargetAlpha[1] = 1.0f;
		return true;
	}
	return false;
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIAnimatedButton::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= UIButton::OnUpdate(dimensionsChanged);

	// If the button is currently in the process of animation, mark the SubPicture as dirty
	for (uint i = 0; i < 2; ++i)
		if (mCurrentAlpha[i] != mTargetAlpha[i])
			return true;

	return dimensionsChanged;
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIAnimatedButton::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mArea == 0 && queue->mTex	== mImage.GetTexture())
	{
		static String faceName[] =
		{
			"Button: Disabled",
			"Button: Enabled",
			"Button: Highlighted",
			"Button: Pressed"
		};

		UIRegion& rgn = mImage.GetRegion();

		if (mState & State::Enabled)
		{
			// Fill the normal face
			mImage.SetFace(faceName[1], false);
			rgn.SetAlpha(1.0f);
			rgn.Update(mRegion);
			mImage.OnFill(queue);

			// Get the current time for animation
			float time = mRoot->GetCurrentTime();

			for (uint i = 0; i < 2; ++i)
			{
				// If alpha is changing, update it
				if ( Float::IsNotEqual(mCurrentAlpha[i], mTargetAlpha[i]) )
				{
					float factor = (mAnimTime > 0.0f) ? Float::Clamp((time - mStartTime[i]) / mAnimTime, 0.0f, 1.0f) : 1.0f;
					mCurrentAlpha[i] = mStartAlpha[i] * (1.0f - factor) + mTargetAlpha[i] * factor;
				}

				// If the secondary face is visible, draw it
				if (mCurrentAlpha[i] > 0.0f)
				{
					mImage.SetFace(faceName[i+2], false);
					rgn.SetAlpha(mCurrentAlpha[i]);
					rgn.Update(mRegion);
					mImage.OnFill(queue);
				}
			}

			// Always finish with region alpha set to 1
			rgn.SetAlpha(1.0f);
			rgn.Update(mRegion);
		}
		else
		{
			mImage.SetFace(faceName[0], false);
			rgn.SetAlpha(1.0f);
			rgn.Update(mRegion);
			mImage.OnFill(queue);
		}
	}
	else mLabel.OnFill(queue);
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIAnimatedButton::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Animation Time")
	{
		node.mValue >> mAnimTime;
		return true;
	}
	return UIButton::OnSerializeFrom(node);
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIAnimatedButton::OnSerializeTo (TreeNode& node) const
{
	UIButton::OnSerializeTo(node);
	node.AddChild("Animation Time", mAnimTime);
}