#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves a single bone by ID, creating it if necessary
//============================================================================================================

Bone* Skeleton::GetBone (uint index, bool createIfMissing)
{
	Bone* bone (0);
	mBones.Lock();
	bone = _GetBone(index, createIfMissing);
	mBones.Unlock();
	return bone;
}

//============================================================================================================
// Finds a bone by its name
//============================================================================================================

Bone* Skeleton::GetBone (const String& name, bool createIfMissing)
{
	mBones.Lock();
	Bone* bone = _GetBone(name, createIfMissing);
	mBones.Unlock();
	return bone;
}

//============================================================================================================
// Read-only bone access
//============================================================================================================

const Bone* Skeleton::GetBone (const String& name) const
{
	const Bone* bone = 0;

	if (mBones.IsValid())
	{
		mBones.Lock();
		{
			for (uint i = mBones.GetSize(); i > 0; )
			{
				if (mBones[--i] != 0 && mBones[i]->GetName() == name)
				{
					bone = mBones[i];
					break;
				}
			}
		}
		mBones.Unlock();
	}
	return bone;
}

//============================================================================================================
// Retrieves the bone's index by its name
//============================================================================================================

uint Skeleton::GetBoneIndex (const String& name) const
{
	uint index = -1;

	if (mBones.IsValid())
	{
		mBones.Lock();
		{
			for (uint i = mBones.GetSize(); i > 0; )
			{
				if (mBones[--i] != 0 && mBones[i]->GetName() == name)
				{
					index = i;
					break;
				}
			}
		}
		mBones.Unlock();
	}
	return index;
}

//============================================================================================================
// Retrieves the animation by its ID
//============================================================================================================

Animation* Skeleton::GetAnimation (uint animID)
{
	uint index = animID & 0xFFFF;

	if (index < mAnims.GetSize())
	{
		mAnims.Lock();
		Animation* anim = mAnims[index];
		mAnims.Unlock();
		if (anim != 0 && anim->GetID() == animID) return anim;
	}
	return 0;
}

//============================================================================================================
// Get the animation by name
//============================================================================================================

Animation* Skeleton::GetAnimation (const String& name, bool createIfMissing)
{
	mAnims.Lock();
	{
		for (uint i = 0; i < mAnims.GetSize(); ++i)
		{
			Animation* anim = mAnims[i];

			if (anim != 0 && anim->GetName() == name)
			{
				mAnims.Unlock();
				return anim;
			}
		}

		if (createIfMissing)
		{
			static uint counter = 0;
			Animation* anim = new Animation(name);
			anim->SetID( (++counter << 16) | mAnims.GetSize() );
			mAnims.Expand() = anim;
			mAnims.Unlock();
			return anim;
		}
	}
	mAnims.Unlock();
	return 0;
}

//============================================================================================================
// Bone retrieval
//============================================================================================================

Bone* Skeleton::_GetBone (uint index, bool createIfMissing)
{
	if (index < 0xFF && createIfMissing)
	{
		mBones.ExpandTo(index+1);
		BonePtr& bone = mBones[index];
		if (bone == 0) bone = new Bone();
		return bone;
	}
	return (index < mBones.GetSize() ? mBones[index] : 0);
}

//============================================================================================================

Bone* Skeleton::_GetBone (const String& name, bool createIfMissing)
{
	for (uint i = 0; i < mBones.GetSize(); ++i)
		if (mBones[i] != 0 && mBones[i]->GetName() == name)
			return mBones[i];

	if (createIfMissing)
	{
		BonePtr& bone = mBones.Expand();
		if (bone == 0) bone = new Bone();
		bone->SetName(name);
		return bone;
	}
	return 0;
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool Skeleton::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];

		if (node.mTag == Bone::ClassName())
		{
			uint index;

			if (node.mValue >> index)
			{
				mBones.Lock();
				{
					Bone* bone = _GetBone(index, true);
					bone->SerializeFrom(node, forceUpdate);
				}
				mBones.Unlock();
			}
		}
		else if (node.mTag == Animation::ClassName())
		{
			GetAnimation(node.mValue.GetString(), true)->SerializeFrom(node);
		}
	}
	return true;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

bool Skeleton::SerializeTo (TreeNode& root) const
{
	if (mBones.IsValid())
	{
		TreeNode& node = root.AddChild("Skeleton", mName);

		mBones.Lock();
		{
			for (uint i = 0; i < mBones.GetSize(); ++i)
			{
				if (mBones[i] != 0)
				{
					TreeNode& bone = node.AddChild(Bone::ClassName(), i);
					mBones[i]->SerializeTo(bone);
				}
			}
		}
		mBones.Unlock();

		mAnims.Lock();
		{
			for (uint i = 0; i < mAnims.GetSize(); ++i)
			{
				Animation* anim (mAnims[i]);
				if (anim != 0) anim->SerializeTo(node);
			}
		}
		mAnims.Unlock();
	}
	return true;
}