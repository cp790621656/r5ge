#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// A list of all scripts that can be created
//============================================================================================================

Hash<UIScript::CreateDelegate> gUIScriptTypes;

//============================================================================================================
// INTERNAL: Registers a new script of the specified type
//============================================================================================================

void UIScript::_Register(const String& type, const CreateDelegate& func)
{
	gUIScriptTypes.Lock();
	gUIScriptTypes[type] = func;
	gUIScriptTypes.Unlock();
}

//============================================================================================================
// INTERNAL: Creates a new script of the specified type
//============================================================================================================

UIScript* UIScript::_Create(const String& type)
{
	UIScript* ptr (0);
	gUIScriptTypes.Lock();
	{
		const CreateDelegate* callback = gUIScriptTypes.GetIfExists(type);
		if (callback != 0) ptr = (*callback)();
	}
	gUIScriptTypes.Unlock();
	return ptr;
}

//============================================================================================================
// Destructor removes this script from the owning widget's list if possible
//============================================================================================================

UIScript::~UIScript()
{
	if (mWidget != 0)
	{
		mWidget->mScripts.Remove(this);
		mWidget = 0;
	}
}

//============================================================================================================
// Destroys this script. The script is never deleted immediately, but is rather scheduled for deletion.
//============================================================================================================

void UIScript::DestroySelf()
{
	if (mWidget != 0)
	{
		mWidget->_DelayedDelete(this);
		mWidget = 0;
	}
}