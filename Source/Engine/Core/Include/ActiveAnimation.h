#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// All active animations need to store additional information that are not part of the referenced animations
//============================================================================================================

struct ActiveAnimation
{
	typedef Array<BoneTransform> BoneTransforms;

	Animation*	mAnimation;			// Pointer to the animation itself
	float		mPlaybackFactor;	// Current playback factor in 0 to 1 range
	float		mSamplingFactor;	// Current playback position in 0 to 1 range, wraps around
	float		mSamplingOffset;	// Offset value used to sync animations
	float		mFadeInEnd;			// 0 to 1 range ending position of when animation becomes 100%
	float		mAnimStart;			// 0 to 1 range position when the animation begins playing
	float		mDurationFactor;	// 0 to 1 range of the animation's duration
	float		mFadeOutStart;		// 0 to 1 range position when the animation begins to fade out
	float		mPlaybackDuration;	// Duration of the entire animation playback in seconds
	float		mOverrideDuration;	// Duration of the user-triggered animation fading
	float		mOverrideFactor;	// Current overriding fading out factor
	float		mCurrentAlpha;		// Current fading factor when fading the frame in 0 to 1 range
	bool		mIsActive;			// Whether this animation is active (internal flag)

	ActiveAnimation();

	// Convenience function
	void Activate (float fadeInFactor, float durationFactor, float fadeOutFactor, float totalDuration);

	// Advance the animation and update the 'mIsActive' flag
	bool AdvanceSample (float delta, const Skeleton::Bones& bones, BoneTransforms& transforms);
};