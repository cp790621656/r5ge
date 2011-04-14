#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// All drawable objects are separated by techniques into different lists
// Author: Michael Lyashenko
//============================================================================================================

class DrawList
{
private:

	PointerHash<DrawGroup> mGroups;
	bool mNeedsSorting;

public:

	DrawList() : mNeedsSorting(false) {}

	// Add a new entry
	void Add (uint group, Object* object, void* param, float distance);

	// Sorts the array
	void Sort() { mNeedsSorting = true; }

	// Clear all entries in the draw list
	void Clear();

	// Draw all objects in the list
	uint Draw (TemporaryStorage& storage, const ITechnique* tech, bool insideOut);
};