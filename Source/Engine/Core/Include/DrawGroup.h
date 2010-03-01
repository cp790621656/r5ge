#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Class managing an array of drawable objects
//============================================================================================================

class DrawGroup
{
public:

	struct Entry
	{
		Object*		mObject;	// Pointer to the object that will be rendered
		float		mDistance;	// Squared distance to the camera, used to sort objects

		// Comparison operator for sorting
		bool operator < (const Entry& obj) const { return (mDistance < obj.mDistance); }
	};

private:

	Array<Entry> mEntries;
	uint mGroup;

public:

	DrawGroup() : mGroup(0) {}

	// Self-identification
	void Set (uint group) { mGroup = group; }

	// Only add this object if it's not already a part of the list
	void Add (Object* object, float distance)
	{
		if (mEntries.IsEmpty() || object != mEntries.Back().mObject)
		{
			Entry& ent		= mEntries.Expand();
			ent.mObject		= object;
			ent.mDistance	= distance;
		}
	}

	void Sort()  { mEntries.Sort();  }
	void Clear() { mEntries.Clear(); }
	uint Draw (const ITechnique* tech, bool insideOut);
};