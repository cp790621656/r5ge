#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// All drawable objects can be placed on different layers which effectively separate the draw process
// Author: Michael Lyashenko
//============================================================================================================

struct DrawLayer
{
	uint mMask;				// Mask used to quickly eliminate the entire layer
	Array<DrawList> mList;	// Different draw lists, one per technique

	// Clear all draw lists
	void Clear();

	// Sort all draw lists
	void Sort();

	// Add the specified object to this layer
	void Add (Object* obj, void* param, uint mask, uint group, float distSquared);

	// Draw the scene
	uint Draw (TemporaryStorage& storage, const ITechnique* tech, bool insideOut);
};