#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Calculated final transform struct for skeletal animation, there needs to be one of these per bone
//============================================================================================================

struct BoneTransform
{
	uint		mParent;		// Index of the parent
	Vector3f	mInvBindPos;	// Inverse bind pose position
	Quaternion	mInvBindRot;	// Inverse bind pose rotation
	Vector3f	mRelativePos;	// Sampled value, relative to the parent transform
	Quaternion	mRelativeRot;
	Vector3f	mAbsolutePos;	// Calculated absolute value in world coordinates
	Quaternion	mAbsoluteRot;
	float		mCombinedPos;	// Calculated combined position alpha
	float		mCombinedRot;

	BoneTransform() : mParent(-1), mCombinedPos(0.0f), mCombinedRot(0.0f) {}

	void CalculateTransformMatrix()
	{
		mAbsolutePos = mRelativePos;
		mAbsoluteRot = mRelativeRot;
	}

	void CalculateTransformMatrix (const BoneTransform& trans)
	{
		mAbsolutePos = mRelativePos * trans.mAbsoluteRot + trans.mAbsolutePos;
		mAbsoluteRot.Combine(trans.mAbsoluteRot, mRelativeRot);
	}
};