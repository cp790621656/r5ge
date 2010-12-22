#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Registered script types
//============================================================================================================

Script::List g_scriptTypes;
Script::List* g_remoteTypes = 0;

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
		Script::Register<OSRotate>();
		Script::Register<OSSceneRoot>();
		Script::Register<OSDrawForward>();
		Script::Register<OSDrawDeferred>();
		Script::Register<OSAudioListener>();
	}
}

//============================================================================================================
// INTERNAL: Registers a new script of the specified type
//============================================================================================================

void Script::_Register (const String& type, const CreateDelegate& func)
{
	_Register(g_remoteTypes == 0 ? g_scriptTypes : *g_remoteTypes, type, func);
}

//============================================================================================================
// INTERNAL: Registers a new script of the specified type
//============================================================================================================

void Script::_Register (List& list, const String& type, const CreateDelegate& func)
{
	if (!list.IsValid()) _RegisterDefaultScripts();

	list.Lock();
	list[type] = func;
	list.Unlock();
}

//============================================================================================================
// INTERNAL: Removes the specified script from the list of registered scripts
//============================================================================================================

void Script::_UnRegister (const String& type)
{
	_UnRegister(g_remoteTypes == 0 ? g_scriptTypes : *g_remoteTypes, type);
}

//============================================================================================================
// INTERNAL: Removes the specified script from the list of registered scripts
//============================================================================================================

void Script::_UnRegister (List& list, const String& type)
{
	if (list.IsValid())
	{
		list.Lock();
		list.Delete(type);
		list.Unlock();
	}
}

//============================================================================================================
// INTERNAL: Creates a new script of the specified type
//============================================================================================================

Script* Script::_Create (const String& type)
{
	return _Create(g_remoteTypes == 0 ? g_scriptTypes : *g_remoteTypes, type);
}

//============================================================================================================
// INTERNAL: Creates a new script of the specified type
//============================================================================================================

Script* Script::_Create (List& list, const String& type)
{
	if (!list.IsValid()) _RegisterDefaultScripts();

	Script* ptr (0);
	list.Lock();
	{
		const CreateDelegate* callback = list.GetIfExists(type);
		if (callback != 0) ptr = (*callback)();
		else WARNING(String("Unknown Script type '%s'", type.GetBuffer()).GetBuffer());
	}
	list.Unlock();
	return ptr;
}

//============================================================================================================
// Sets the replacement script type list that should be used instead of the built-in one
//============================================================================================================

void Script::SetTypeList (List* list)
{
	g_remoteTypes = list;
}

//============================================================================================================
// Retrieves the local type list
//============================================================================================================

Script::List* Script::GetTypeList()
{
	return &g_scriptTypes;
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