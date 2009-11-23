#include "../Include/_All.h"
using namespace R5;

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