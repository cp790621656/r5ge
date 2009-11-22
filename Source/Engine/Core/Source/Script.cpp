#include "../Include/_All.h"

using namespace R5;

//============================================================================================================
// Constructor tries to load the script if it can
//============================================================================================================

Script::Script(const String& name) : mName(name), mSerializable(false)
{
	// If the script loads from the simple filename, no need to serialize it
	if (Load(name)) mSerializable = false;
}

//============================================================================================================
// Loads a script from source
//============================================================================================================

bool Script::Load (const char* source)
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

bool Script::SerializeFrom (const TreeNode& root, bool forceUpdate)
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
				Load(value.IsString() ? value.AsString() : value.GetString());
			}
		}
		else if (tag == "Serializable") value >> mSerializable;
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Script::SerializeTo (TreeNode& root) const
{
	if (IsSerializable())
	{
		TreeNode& node = root.AddChild("Script", mName);
		node.AddChild("Source", mSource);
	}
	return true;
}