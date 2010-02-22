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
// Destroys this script - this action is queued until next update
//============================================================================================================

void Script::DestroySelf()
{
	if (mObject != 0)
	{
		mObject->mScripts.Remove(this);
		mObject->mDeletedScripts.Expand() = this;
		mObject = 0;
	}
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