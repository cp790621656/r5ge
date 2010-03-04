#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

ActiveAnimation::ActiveAnimation() :
	mAnimation			(0),
	mPlaybackFactor		(0.0f),
	mSamplingFactor		(0.0f),
	mSamplingOffset		(0.0f),
	mFadeInEnd			(0.0f),
	mAnimStart			(0.0f),
	mDurationFactor		(1.0f),
	mFadeOutStart		(1.0f),
	mOverrideDuration	(0.0f),
	mOverrideFactor		(0.0f),
	mPlaybackDuration	(0.0f),
	mCurrentAlpha		(0.0f),
	mStrength			(1.0f),
	mIsActive			(false)
{
}

//============================================================================================================
// Convenience function
//============================================================================================================

void ActiveAnimation::Activate (
	float fadeInFactor,
	float durationFactor,
	float fadeOutFactor,
	float totalDuration,
	float strength)
{
	mPlaybackFactor		= 0.0f;
	mSamplingFactor		= 0.0f;
	mSamplingOffset		= 0.0f;
	mFadeInEnd			= fadeInFactor;
	mAnimStart			= 0.5f - 0.5f * durationFactor;
	mDurationFactor		= durationFactor;
	mFadeOutStart		= 1.0f - fadeOutFactor;
	mOverrideDuration	= 0.0f;
	mOverrideFactor		= 0.0f;
	mPlaybackDuration	= totalDuration;
	mCurrentAlpha		= 0.0f;
	mStrength			= strength;
	mIsActive			= true;
}

//============================================================================================================
// Advances the animation sample by the specified delta time
//============================================================================================================

bool ActiveAnimation::AdvanceSample (float delta, const Skeleton::Bones& bones, BoneTransforms& transforms)
{
	bool isSolid = false;

	delta = Float::Abs(delta);

	if (delta != 0.0f && mAnimation != 0 && bones.IsValid())
	{
		if (mPlaybackDuration == 0.0f)
		{
			// If the playback duration has not been set, there is nothing to advance
			mSamplingFactor = 0.0f;
			mIsActive = false;
		}
		else
		{
			// Advance the playback
			mPlaybackFactor += delta / mPlaybackDuration;

			// If the playback has reached the end, we need to either wrap it around or stop it
			if (mPlaybackFactor >= 1.0f)
			{
				if (mAnimation->IsLooping())
				{
					mFadeInEnd		= 0.0f;
					mAnimStart		= 0.0f;
					mDurationFactor	= 1.0f;
					mFadeOutStart	= 1.0f;
					mPlaybackFactor	= Float::Fract(mPlaybackFactor);
				}
				else
				{
					mPlaybackFactor = 1.0f;
					mIsActive = false;
				}
			}
			else if (mPlaybackFactor < 0.0f)
			{
				// Safety precaution more than anything
				mPlaybackFactor = 0.0f;
			}

			// Figure out the updated alpha
			if (mPlaybackFactor < mFadeInEnd)
			{
				ASSERT(mFadeInEnd != 0.0f, "Fade in end should have been above 0");

				// We're fading in the animation
				mCurrentAlpha = 1.0f - (mFadeInEnd - mPlaybackFactor) / mFadeInEnd;
				mCurrentAlpha = Interpolation::Cosine(0.0f, 1.0f, mCurrentAlpha);
			}
			else if (mPlaybackFactor > mFadeOutStart)
			{
				ASSERT(mFadeOutStart != 1.0f, "Fade out start should have been under 1");

				// We're fading out the animation
				mCurrentAlpha = 1.0f - (mPlaybackFactor - mFadeOutStart) / (1.0f - mFadeOutStart);
				mCurrentAlpha = Interpolation::Cosine(0.0f, 1.0f, mCurrentAlpha);
			}
			else
			{
				// We're fully inside the animation
				mCurrentAlpha = 1.0f;
			}

			// If we have an override value used (set by Model::StopAnimation), use it
			if (mOverrideDuration != 0.0f)
			{
				// Choose the least of the two
				mOverrideFactor += delta / mOverrideDuration;
				mCurrentAlpha = Float::Min(mCurrentAlpha, 1.0f - mOverrideFactor);

				// Deactivate this animation if necessary
				if (mCurrentAlpha < 0.0f)
				{
					mCurrentAlpha = 0.0f;
					mIsActive = false;
				}
				else if (mCurrentAlpha > 1.0f)
				{
					mCurrentAlpha = 1.0f;
					mOverrideDuration = 0.0f;
				}
			}

			// If the animation has no duration, just leave sampling at 0
			if (mDurationFactor == 0.0f)
			{
				mSamplingFactor = 0.0f;
			}
			else
			{
				ASSERT(mDurationFactor != 0.0f, "Animation factor is zero?");

				// The animation has a duration -- determine where in the animation we should sample it
				mSamplingFactor = (mPlaybackFactor - mAnimStart) / mDurationFactor;

				if (mAnimation->IsLooping())
				{
					mSamplingFactor = Float::Fract(mSamplingFactor + mSamplingOffset);
				}
				else
				{
					mSamplingFactor = Float::Clamp(mSamplingFactor, 0.0f, 1.0f);
				}
			}
		}

		if (mCurrentAlpha > 0.0f)
		{
			if (mAnimation->IsDirty())
			{
				// If the animation has no bones, fill them in
				mAnimation->Fill(bones);
			}

			// Values used for sampling
			uint result = 0;
			Vector3f pos;
			Quaternion rot;

			// Reset the 'solid' flag to true by default unless proven otherwise below
			isSolid = true;

			// Run through all transforms and sample them at the current time
			for (uint i = transforms.GetSize(); i > 0; )
			{
				BoneTransform& trans = transforms[--i];

				bool needsPos = (trans.mCombinedPos != 1.0f);
				bool needsRot = (trans.mCombinedRot != 1.0f);

				if (needsPos || needsRot)
				{
					// Sample the animation with the current factor
					result = mAnimation->Sample(i, mSamplingFactor, pos, rot);

					// If we can contribute a position
					if (needsPos && (result & 0x1) != 0)
					{
						// Interpolate the value between our current and what's already there
						float factor		= mCurrentAlpha * (1.0f - trans.mCombinedPos) * mStrength;
						trans.mRelativePos  = Interpolation::Linear(trans.mRelativePos, pos, factor);
						trans.mCombinedPos += mCurrentAlpha;
					}

					// If we can contribute rotation
					if (needsRot && (result & 0x2) != 0)
					{
						float factor		= mCurrentAlpha * (1.0f - trans.mCombinedRot) * mStrength;
						trans.mRelativeRot  = Interpolation::Slerp(trans.mRelativeRot, rot, factor);
						trans.mCombinedRot += mCurrentAlpha;
					}

					// Update the solidity flag and ensure the values don't exceed 1.0
					if ( trans.mCombinedPos < 1.0f ) isSolid = false;
					else trans.mCombinedPos = 1.0f;
					if ( trans.mCombinedRot < 1.0f ) isSolid = false;
					else trans.mCombinedRot = 1.0f;
				}
			}
		}
	}
	return isSolid;
}