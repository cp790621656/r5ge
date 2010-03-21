#pragma once

//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Update 
//============================================================================================================

class OSSound : public Script
{
	ISoundInstance* mSound;
	String			mName;
	uint			mLayer;

public:

	R5_DECLARE_INHERITED_CLASS("OSSound", OSSound, Script, Script);

	OSSound() :
		mSound (0) {}

	virtual void OnUpdate()
	{
		if (mSound == 0)
		{
			mSound = mObject->GetCore()->GetAudio()->GetSound(mName)->Play(mObject->GetAbsolutePosition(), mLayer);
		}

		if (mSound != 0)
		{
			mSound->SetPosition(mObject->GetAbsolutePosition());
		}
	}

	//============================================================================================================
	// Serilization -- Save
	//============================================================================================================

	virtual void OnSerializeTo(TreeNode& node) const
	{
		node.AddChild("Sound", mName);
		node.AddChild("Layer", mLayer);
	}

	//============================================================================================================
	// Serilization -- Load
	//============================================================================================================

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