#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// All drawable objects can be placed on different layers which effectively separate the draw process
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
	void Add (Object* obj, uint mask, uint group, float distSquared);

	// Draw the scene
	uint Draw (const Deferred::Storage& storage, const ITechnique* tech, bool insideOut);
};