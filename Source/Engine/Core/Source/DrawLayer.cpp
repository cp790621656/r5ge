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
		mList[--i].Clear();
	}
}

//============================================================================================================
// Sort all draw lists
//============================================================================================================

void DrawLayer::Sort()
{
	for (uint i = mList.GetSize(); i > 0; )
	{
		mList[--i].Sort();
	}
}

//============================================================================================================
// Add the specified object to this layer
//============================================================================================================

void DrawLayer::Add (Object* obj, uint mask, uint group, float distSquared)
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
			mList[i].Add(group, obj, distSquared);
		}
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

uint DrawLayer::Draw (const Deferred::Storage& storage, const ITechnique* tech, bool insideOut)
{
	uint index (tech->GetIndex()), retVal (0);

	if (index < mList.GetSize())
	{
		retVal += mList[index].Draw(storage, tech, insideOut);
	}
	return retVal;
}