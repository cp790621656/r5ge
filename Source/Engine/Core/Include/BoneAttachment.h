#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Script that binds its owner to the specified bone of the owner's parent
//============================================================================================================

class BoneAttachment : public Script
{
	String mName;
	uint mBoneIndex;

public:

	BoneAttachment() : mBoneIndex(-1) {}

	R5_DECLARE_INHERITED_CLASS("Bone Attachment", BoneAttachment, Script, Script);

	// Reset the index of the bone, forcing it to be found again by its name next Update
	void Reset() { mBoneIndex = -1; }

	// Set the bone by name
	void SetBone (const String& name) { mName = name; mBoneIndex = -1; }

	// The owner's absolute coordinates need to be calculated in the update
	virtual void OnUpdate();

	// Serialization
	virtual bool SerializeTo	(TreeNode& root) const;
	virtual bool SerializeFrom	(const TreeNode& root);
};