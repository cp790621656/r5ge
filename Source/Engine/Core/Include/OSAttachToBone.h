#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Script that binds its owner to the specified bone of the owner's parent
//============================================================================================================

class OSAttachToBone : public Script
{
	String mName;
	uint mBoneIndex;

public:

	OSAttachToBone() : mBoneIndex(-1) {}

	R5_DECLARE_INHERITED_CLASS("OSAttachToBone", OSAttachToBone, Script, Script);

	// Reset the index of the bone, forcing it to be found again by its name next Update
	void Reset() { mBoneIndex = -1; }

	// Set the bone by name
	void SetBone (const String& name) { mName = name; mBoneIndex = -1; }

	// The owner's absolute coordinates need to be calculated in the update
	virtual void OnUpdate();

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const;
	virtual void OnSerializeFrom(const TreeNode& node);
};