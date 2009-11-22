#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Adds a new OnKey event listener
//============================================================================================================

void UpdateList::Add (const Callback& callback, float delay)
{
	mUpdate.Lock();
	{
		UpdateEntry& entry = mUpdate.Expand();
		entry.time = Time::GetTime() + delay;
		entry.callback = callback;
	}
	mUpdate.Unlock();
}

//============================================================================================================
// Runs through all update listeners and calls them as necessary
//============================================================================================================

void UpdateList::Execute()
{
	mUpdate.Lock();
	{
		float time = Time::GetTime(), result;

		for (uint i = mUpdate.GetSize(); i > 0; )
		{
			UpdateEntry& entry = mUpdate[--i];
			if (entry.time > time) continue;

			result = entry.callback();

			if (result < 0.0f)
			{
				mUpdate.RemoveAt(i);
			}
			else
			{
				entry.time = time + result;
			}
		}
	}
	mUpdate.Unlock();
}