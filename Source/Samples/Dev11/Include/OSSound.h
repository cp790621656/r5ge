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
	ISoundInstance* mInst;
	String			mName;
	uint			mLayer;

public:

	R5_DECLARE_INHERITED_CLASS("OSSound", OSSound, Script, Script);

	OSSound() : mInst (0), mLayer(0) {}

	// Update the sound's position
	virtual void OnUpdate()
	{
		if (mInst == 0)
		{
			if (mName.IsEmpty()) return;
			const Vector3f& pos = mObject->GetAbsolutePosition();
			ISound* sound = mObject->GetCore()->GetAudio()->GetSound(mName);
			mInst = sound->Play(pos, mLayer, 0.1f, true);
			mInst->SetEffect(ISoundInstance::Effect::Auditorium);
		}

		if (mInst != 0)
		{
			const Vector3f& pos = mObject->GetAbsolutePosition();
			mInst->SetPosition(pos);
		}
	}

	// Serilization -- Save
	virtual void OnSerializeTo(TreeNode& node) const
	{
		node.AddChild("Sound", mName);
		node.AddChild("Layer", mLayer);
	}

	// Serilization -- Load
	virtual void OnSerializeFrom(const TreeNode& node)
	{
		if (node.mTag == "Sound")
		{
			mName = node.mValue.AsString();
		}
		else if (node.mTag == "Layer")
		{
			mLayer = node.mValue.AsUInt();
		}
	}
};