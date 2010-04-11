#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Registered script types
//============================================================================================================

Hash<Script::CreateDelegate> g_scriptTypes;

//============================================================================================================
// Registers common scripts that belong to the Core
//============================================================================================================

void _RegisterDefaultScripts()
{
	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;
		Script::Register<OSAttachToBone>();
		Script::Register<OSPlayAnimations>();
		Script::Register<OSPlayIdleAnimations>();
	}
}

//============================================================================================================
// INTERNAL: Registers a new script of the specified type
//============================================================================================================

void Script::_Register(const String& type, const CreateDelegate& func)
{
	if (!g_scriptTypes.IsValid()) _RegisterDefaultScripts();

	g_scriptTypes.Lock();
	g_scriptTypes[type] = func;
	g_scriptTypes.Unlock();
}

//============================================================================================================
// INTERNAL: Creates a new script of the specified type
//============================================================================================================

Script* Script::_Create(const String& type)
{
	if (!g_scriptTypes.IsValid()) _RegisterDefaultScripts();

	Script* ptr (0);
	g_scriptTypes.Lock();
	{
		const CreateDelegate* callback = g_scriptTypes.GetIfExists(type);
		if (callback != 0) ptr = (*callback)();
	}
	g_scriptTypes.Unlock();
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

void Script::DestroySelf (bool threadSafe)
{
	if (mObject != 0)
	{
		OnDestroy();

		if (threadSafe) mObject->Lock();
		{
			mObject->mScripts.Remove(this);
			mObject->mDeletedScripts.Expand() = this;
		}
		if (threadSafe) mObject->Unlock();
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