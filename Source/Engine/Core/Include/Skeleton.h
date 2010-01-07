#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Skeleton class used for skinning animation
//============================================================================================================

class Skeleton
{
public:

	typedef Bone*						BonePtr;
	typedef Animation::Bones			Bones;
	typedef ResourceArray<Animation>	Animations;

private:

	String		mName;		// Name of the skeleton
	Bones		mBones;		// List of associated bones
	Animations	mAnims;		// List of known animations

public:

	// Static identifier, for consistency
	R5_DECLARE_SOLO_CLASS("Skeleton");

	Skeleton(const String& name) : mName(name) {}

public:

	bool IsValid() const	{ return mBones.IsValid(); }
	void Release()			{ mBones.Lock(); mBones.Release(); mBones.Unlock(); }

	void				SetName(const String& name)	{ mName = name;		}
	const String&		GetName()			const	{ return mName;		}
	Bones&				GetAllBones()				{ return mBones;	}
	const Bones&		GetAllBones()		const	{ return mBones;	}
	Animations&			GetAllAnimations()			{ return mAnims;	}
	const Animations&	GetAllAnimations()	const	{ return mAnims;	}

	// Bone creation and retrieval
	Bone* GetBone (uint index, bool createIfMissing = false);
	Bone* GetBone (const String& name, bool createIfMissing = false);

	// Read-only bone access
	const Bone* GetBone(const String& name) const;

	// Retrieves the bone's index by its name
	uint GetBoneIndex(const String& name) const;

	// Animation creation and retrieval
	Animation* GetAnimation (uint animID);
	Animation* GetAnimation (const String& name, bool createIfMissing = true);

private:

	Bone* _GetBone (uint index, bool createIfMissing = false);
	Bone* _GetBone (const String& name,  bool createIfMissing = false);

public:

	// Serialization
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& root) const;
};