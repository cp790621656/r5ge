#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Draw all objects in the list
//============================================================================================================

uint DrawList::Draw (const ITechnique* tech, bool insideOut)
{
	uint result (0);

	if (tech->GetSorting() == ITechnique::Sorting::BackToFront)
	{
		for (uint b = mEntries.GetSize(); b > 0; )
		{
			result += mEntries[--b].mObject->OnDraw(tech, insideOut);
		}
	}
	else
	{
		for (uint i = 0, imax = mEntries.GetSize(); i < imax; ++i)
		{
			result += mEntries[i].mObject->OnDraw(tech, insideOut);
		}
	}
	return result;
}