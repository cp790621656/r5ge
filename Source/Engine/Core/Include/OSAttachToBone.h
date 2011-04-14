#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Script that binds its owner to the specified bone of the owner's parent
// Author: Michael Lyashenko
//============================================================================================================

class OSAttachToBone : public Script
{
protected:

	String mName;
	uint mBoneIndex;

	// Use the AddScript<> template to add new scripts
	OSAttachToBone() : mBoneIndex(-1) {}

public:

	R5_DECLARE_INHERITED_CLASS("OSAttachToBone", OSAttachToBone, Script, Script);

	// Reset the index of the bone, forcing it to be found again by its name next Update
	void Reset() { mBoneIndex = -1; }

	// Name of the associated bone
	const String& GetBone() const { return mName; }

	// Set the bone by name
	void SetBone (const String& name) { mName = name; mBoneIndex = -1; }

	// The owner's absolute coordinates need to be calculated in the update
	virtual void OnUpdate();

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const;
	virtual void OnSerializeFrom(const TreeNode& node);
};