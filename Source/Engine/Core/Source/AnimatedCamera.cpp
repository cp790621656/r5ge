#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Animate to the specified relative orientation over the specified amount of time
//============================================================================================================

void AnimatedCamera::AnimateTo (const Vector3f& pos, const Quaternion& rot, float fov, float delay)
{
	if (mV.IsEmpty())
	{
		float time = Time::GetTime();
		mV.AddKey(time, mRelativePos);
		mQ.AddKey(time, mRelativeRot);
		mF.AddKey(time, mRelativeRange.z);
	}
	mV.AddKey(mV.EndTime() + delay, pos);
	mQ.AddKey(mQ.EndTime() + delay, rot);
	mF.AddKey(mF.EndTime() + delay, fov);
}

//============================================================================================================
// Modify the relative coordinates based on sampled splines
//============================================================================================================

void AnimatedCamera::OnPreUpdate()
{
	if (mV.IsValid())
	{
		float time = Time::GetTime();
		mRelativePos = mV.Sample(time);
		mRelativeRot = mQ.Sample(time);
		mRelativeRange.z = mF.Sample(time);
		mIsDirty = true;
	}
}