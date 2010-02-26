#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Thread-safe function that can be used to generate ever-incrementing unique identifiers
//============================================================================================================

uint R5::GenerateUID()
{
	static Thread::Lockable lock;
	static uint uid = 0;

	lock.Lock();
	uint retVal = ++uid;
	lock.Unlock();
	return retVal;
}