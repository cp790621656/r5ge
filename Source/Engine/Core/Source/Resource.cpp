#include "../Include/_All.h"

using namespace R5;

//============================================================================================================
// Constructor tries to load the resource if it can
//============================================================================================================

Resource::Resource(const String& name) : mName(name), mSerializable(false)
{
	// If the resource loads from the simple filename, no need to serialize it
	if (Load(name)) mSerializable = false;
}

//============================================================================================================
// Loads a resource from source
//============================================================================================================

bool Resource::Load (const char* source)
{
	mContents.Release();

	if ( mContents.Load(source) )
	{
		mSerializable = true;
		mSource = source;
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Resource::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if (tag == "Source")
		{
			if (!IsValid() || forceUpdate)
			{
				Load(value.AsString());
			}
		}
		else if (tag == "Serializable") value >> mSerializable;
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Resource::SerializeTo (TreeNode& root) const
{
	if (IsSerializable())
	{
		TreeNode& node = root.AddChild(Resource::ClassID(), mName);
		node.AddChild("Source", mSource);
	}
	return true;
}