#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Helper function that parses the bone keyframe information for positions
//============================================================================================================

bool ParsePositionFrames (Bone& bone, const TreeNode& root)
{
	const Array<ushort>* keys = 0;
	const Array<Vector3f>* values = 0;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	= root.mChildren[i];
		const String&	tag		= node.mTag;
		const Variable& value	= node.mValue;

		if (tag == "Smooth")
		{
			bool val;
			if (value >> val) bone.SetUseSplinesForPositions(val);
		}
		else if (tag == "Frames")
		{
			if (value.IsUShortArray())
			{
				keys = &value.AsUShortArray();
			}
		}
		else if (tag == "Values")
		{
			if (value.IsVector3fArray())
			{
				values = &value.AsVector3fArray();
			}
		}
	}

	if (keys != 0 && values != 0)
	{
		if (keys->GetSize() == values->GetSize())
		{
			for (uint i = 0, imax = keys->GetSize(); i < imax; ++i)
			{
				Bone::PosKey* key = bone.GetPosKey((*keys)[i], true);
				key->mPos = (*values)[i];
			}
			return true;
		}
	}
	ASSERT(false, "Failed to parse position keyframe information");
	return false;
}

//============================================================================================================
// Helper function that parses the bone keyframe information for rotations
//============================================================================================================

bool ParseRotationFrames (Bone& bone, const TreeNode& root)
{
	const Array<ushort>* keys = 0;
	const Array<Quaternion>* values = 0;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	= root.mChildren[i];
		const String&	tag		= node.mTag;
		const Variable& value	= node.mValue;

		if (tag == "Smooth")
		{
			bool val;
			if (value >> val) bone.SetUseSplinesForRotations(val);
		}
		else if (tag == "Frames")
		{
			if (value.IsUShortArray())
			{
				keys = &value.AsUShortArray();
			}
		}
		else if (tag == "Values")
		{
			if (value.IsQuaternionArray())
			{
				values = &value.AsQuaternionArray();
			}
		}
	}

	if (keys != 0 && values != 0)
	{
		if (keys->GetSize() == values->GetSize())
		{
			for (uint i = 0, imax = keys->GetSize(); i < imax; ++i)
			{
				Bone::RotKey* key = bone.GetRotKey((*keys)[i], true);
				key->mRot = (*values)[i];
			}
			return true;
		}
	}
	ASSERT(false, "Failed to parse rotation keyframe information");
	return false;
}

//============================================================================================================
// Releases all keys
//============================================================================================================

void Bone::Release()
{
	mParent = -1;
	mPosKeys.Release();
	mRotKeys.Release();
}

//============================================================================================================
// Retrieves the first usable keyframe
//============================================================================================================

uint Bone::GetFirstFrame() const
{
	uint posKey = mPosKeys.IsValid() ? mPosKeys.Front().mFrame : 0;
	uint rotKey = mRotKeys.IsValid() ? mRotKeys.Front().mFrame : 0;
	return (posKey < rotKey ? posKey : rotKey);
}

//============================================================================================================
// Retrieves the last usable keyframe
//============================================================================================================

uint Bone::GetLastFrame() const
{
	uint posKey = mPosKeys.IsValid() ? mPosKeys.Back().mFrame : 0;
	uint rotKey = mRotKeys.IsValid() ? mRotKeys.Back().mFrame : 0;
	return (posKey > rotKey ? posKey : rotKey);
}

//============================================================================================================
// Retrieves or creates a key at the specified frame
//============================================================================================================

Bone::PosKey* Bone::GetPosKey (uint frame, bool createIfMissing)
{
	for (uint i = 0; i < mPosKeys.GetSize(); ++i)
	{
		PosKey& key = mPosKeys[i];

		if (key.mFrame  < frame) continue;
		if (key.mFrame == frame) return &key;
		break;
	}

	if (createIfMissing)
	{
		mPosKeys.Expand().mFrame = frame;
		mPosKeys.Sort();
		return GetPosKey(frame, false);
	}
	return 0;
}

//============================================================================================================

Bone::RotKey* Bone::GetRotKey (uint frame, bool createIfMissing)
{
	for (uint i = 0; i < mRotKeys.GetSize(); ++i)
	{
		RotKey& key = mRotKeys[i];

		if (key.mFrame  < frame) continue;
		if (key.mFrame == frame) return &key;
		break;
	}

	if (createIfMissing)
	{
		mRotKeys.Expand().mFrame = frame;
		mRotKeys.Sort();
		return GetRotKey(frame, false);
	}
	return 0;
}

//============================================================================================================
// Deletes a key at the specified frame
//============================================================================================================

bool Bone::DeletePosKey (uint frame)
{
	for (uint i = 0; i < mPosKeys.GetSize(); ++i)
	{
		uint thisFrame = mPosKeys[i].mFrame;

		if (thisFrame < frame) continue;
		if (thisFrame == frame)
		{
			mPosKeys.RemoveAt(i);
			return true;
		}
		break;
	}
	return false;
}

//============================================================================================================

bool Bone::DeleteRotKey (uint frame)
{
	for (uint i = 0; i < mRotKeys.GetSize(); ++i)
	{
		uint thisFrame = mRotKeys[i].mFrame;

		if (thisFrame < frame) continue;
		if (thisFrame == frame)
		{
			mRotKeys.RemoveAt(i);
			return true;
		}
		break;
	}
	return false;
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool Bone::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	= root.mChildren[i];
		const String&	tag		= node.mTag;
		const Variable& value	= node.mValue;

		if (tag == "Name")
		{
			value >> mName;
		}
		else if (tag == "Parent")
		{
			value >> mParent;
		}
		else if (tag == "Position")
		{
			value >> mPos;
			if (node.HasChildren()) ParsePositionFrames(*this, node);
		}
		else if (tag == "Rotation")
		{
			value >> mRot;
			if (node.HasChildren()) ParseRotationFrames(*this, node);
		}
	}
	return true;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

bool Bone::SerializeTo (TreeNode& node) const
{
	ASSERT(node.mTag == Bone::ClassID(), "Missing 'Bone = ID' tag!");

	if (mParent != -1) node.AddChild("Parent", mParent);
	node.AddChild("Name", mName);

	TreeNode& pos = node.AddChild("Position", mPos);
	TreeNode& rot = node.AddChild("Rotation", mRot);

	if (mPosKeys.IsValid())
	{
		pos.AddChild("Smooth", mSmoothPos);

		Array<ushort>& frames = pos.AddChild("Frames").mValue.ToUShortArray();
		Array<Vector3f>& values = pos.AddChild("Values").mValue.ToVector3fArray();

		for (uint i = 0; i < mPosKeys.GetSize(); ++i)
		{
			const PosKey& key = mPosKeys[i];
			frames.Expand() = key.mFrame;
			values.Expand() = key.mPos;
		}
	}

	if (mRotKeys.IsValid())
	{
		rot.AddChild("Smooth", mSmoothRot);

		Array<ushort>& frames = rot.AddChild("Frames").mValue.ToUShortArray();
		Array<Quaternion>& values = rot.AddChild("Values").mValue.ToQuaternionArray();

		for (uint i = 0; i < mRotKeys.GetSize(); ++i)
		{
			const RotKey& key = mRotKeys[i];
			frames.Expand() = key.mFrame;
			values.Expand() = key.mRot;
		}
	}
	return true;
}