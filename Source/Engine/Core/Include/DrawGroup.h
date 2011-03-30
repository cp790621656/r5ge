#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
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
		void*		mParam;		// Optional parameter passed along with the draw call
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
	void Add (Object* object, void* param, float distance)
	{
		if (mEntries.IsValid() &&
			object == mEntries.Back().mObject &&
			param  == mEntries.Back().mParam) return;

		Entry& ent		= mEntries.Expand();
		ent.mObject		= object;
		ent.mParam		= param;
		ent.mDistance	= distance;
	}

	void Sort()  { mEntries.Sort();  }
	void Clear() { mEntries.Clear(); }
	uint Draw (TemporaryStorage& storage, const ITechnique* tech, bool insideOut);
};