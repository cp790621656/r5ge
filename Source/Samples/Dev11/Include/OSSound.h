#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Positional sound test script
// Author: Michael Lyashenko
//============================================================================================================

class OSSound : public Script
{
	IAudio*			mAudio;
	ISound*			mSound;
	ISoundInstance* mInst;
	uint			mLayer;

public:

	R5_DECLARE_INHERITED_CLASS("OSSound", OSSound, Script, Script);

	OSSound() : mAudio(0), mSound(0), mInst(0), mLayer(0) {}

	virtual void OnInit()
	{
		mAudio = mObject->GetCore()->GetAudio();
		ASSERT(mAudio != 0, "Missing the audio");
	}

	// Update the sound's position
	virtual void OnUpdate()
	{
		if (!mInst)
		{
			if (mSound != 0 && mSound->IsValid())
			{
				mInst = mSound->Play(mObject->GetAbsolutePosition(), mLayer, 0.1f, true);
				mInst->SetEffect(ISoundInstance::Effect::Auditorium);
			}
		}
		else
		{
			mInst->SetPosition(mObject->GetAbsolutePosition());
		}
	}

	// Serialization -- Save
	virtual void OnSerializeTo(TreeNode& node) const
	{
		if (mSound != 0) node.AddChild("Sound", mSound->GetName());
		node.AddChild("Layer", mLayer);
	}

	// Serialization -- Load
	virtual void OnSerializeFrom(const TreeNode& node)
	{
		if (node.mTag == "Sound")
		{
			mSound = mAudio->GetSound(node.mValue.AsString());
		}
		else if (node.mTag == "Layer")
		{
			mLayer = node.mValue.AsUInt();
		}
	}
};