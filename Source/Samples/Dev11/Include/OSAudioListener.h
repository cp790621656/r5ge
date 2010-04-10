#pragma once

//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. / Philip Cosgrave All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Audio Listener scripts updates the sound listener position every frame
//============================================================================================================

class OSAudioListener : public Script
{
	IAudio* mAudio;
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