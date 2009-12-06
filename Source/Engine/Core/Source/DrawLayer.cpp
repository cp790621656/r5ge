#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Clear all draw lists
//============================================================================================================

void DrawLayer::Clear()
{
	mMask = 0;

	for (uint i = mList.GetSize(); i > 0; )
	{
		mList[--i].mEntries.Clear();
	}
}

//============================================================================================================
// Sort all draw lists
//============================================================================================================

void DrawLayer::Sort()
{
	for (uint i = mList.GetSize(); i > 0; )
	{
		mList[--i].mEntries.Sort();
	}
}

//============================================================================================================
// Add the specified object to this layer
//============================================================================================================

void DrawLayer::Add (Object* obj, uint mask, const void* group, float distSquared)
{
	mMask |= mask;

	for (uint i = 0; i < 32; ++i)
	{
		// Bit shift the mask to the technique bit
		uint flag = (mask >> i);

		// If the flag has no other bits set, we're done
		if (flag == 0) break;

		// If this technique bit is set, add this object to the list
		if ((flag & 0x1) != 0)
		{
			mList.ExpandTo(i + 1);

			DrawEntry& drawable = mList[i].mEntries.Expand();
			drawable.mObject = obj;
			drawable.mGroup = group;
			drawable.mDistSquared = distSquared;
		}
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

uint DrawLayer::Draw (const ITechnique* tech, bool insideOut)
{
	uint index = tech->GetIndex();

	if (index < mList.GetSize())
	{
		DrawList& list = mList[index];
		
		if (list.mEntries.IsValid())
		{
			return list.Draw(tech, insideOut);
		}
	}
	return 0;
}