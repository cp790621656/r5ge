#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Registered script types
//============================================================================================================

Hash<Script::CreateDelegate> gScriptTypes;

//============================================================================================================
// INTERNAL: Registers a new script of the specified type
//============================================================================================================

void Script::_Register(const String& type, const CreateDelegate& func)
{
	gScriptTypes.Lock();
	gScriptTypes[type] = func;
	gScriptTypes.Unlock();
}

//============================================================================================================
// INTERNAL: Creates a new script of the specified type
//============================================================================================================

Script* Script::_Create(const String& type)
{
	Script* ptr (0);
	gScriptTypes.Lock();
	{
		const CreateDelegate* callback = gScriptTypes.GetIfExists(type);
		if (callback != 0) ptr = (*callback)();
	}
	gScriptTypes.Unlock();
	return ptr;
}

//============================================================================================================
// Destructor removes this script from the owning object's list if possible
//============================================================================================================

Script::~Script()
{
	if (mObject != 0)
	{
		mObject->mScripts.Remove(this);
		mObject = 0;
	}
}

//============================================================================================================
// Convenience function: mObject->GetCore()->IsKeyDown(key);
//============================================================================================================

bool Script::IsKeyDown (uint key)
{
	return mObject->GetCore()->IsKeyDown(key);
}

//============================================================================================================
// Destroys this script - this action is queued until next update
//============================================================================================================

void Script::DestroySelf()
{
	if (mObject != 0)
	{
		OnDestroy();
		mObject->mScripts.Remove(this);
		mObject->mDeletedScripts.Expand() = this;
		mObject = 0;
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Script::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild(Script::ClassID(), GetClassID());
		OnSerializeTo(node);
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void Script::SerializeFrom (const TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		OnSerializeFrom(root.mChildren[i]);
	}
}