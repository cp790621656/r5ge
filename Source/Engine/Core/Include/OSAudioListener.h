#pragma once

//============================================================================================================
//       R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko and Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Updates the sound listener position every update (attach to your main camera)
//============================================================================================================

class OSAudioListener : public Script
{
protected:

	IAudio* mAudio;

	OSAudioListener() : mAudio(0) {}

public:

	R5_DECLARE_INHERITED_CLASS("OSAudioListener", OSAudioListener, Script, Script);

	virtual void OnInit()
	{
		mAudio = mObject->GetCore()->GetAudio();
		if (mAudio == 0) DestroySelf();
	}

	virtual void OnUpdate()
	{
		const Quaternion& rot = mObject->GetAbsoluteRotation();
		mAudio->SetListener(mObject->GetAbsolutePosition(), rot.GetForward(), rot.GetUp());
	}
};