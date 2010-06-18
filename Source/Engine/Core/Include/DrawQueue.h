#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// All drawable objects are separated by layers into different draw sets.
//------------------------------------------------------------------------------------------------------------
// The draw hierarchy is as follows:
//------------------------------------------------------------------------------------------------------------
// - DrawQueue contains up to 32 DrawLayers, as objects can be placed on different layers.
//   This allows you to manually control the draw order of certain objects.
// - Each layer is split up into multiple DrawLists by their associated draw techniques.
//   This minimizes the number of state switches between draw operations.
// - Each DrawList is in turn split up into different DrawGroups by the group identifier.
//   This allows batching of similar objects together -- instanced models, particles, etc.
// - Each DrawGroup contains a list of objects that belong to it along with their distance to the camera.
//   Distance allows objects to be drawn back-to-front or front-to-back as needed.
//============================================================================================================

class DrawQueue
{
public:

	typedef Array<const ITechnique*> Techniques;

private:

	// Only the Scene class should be touching 'mLayers' directly
	friend class Scene;

	Light::List	mLights;
	DrawLayer	mLayers[32];

public:

	// Clear the draw queue
	void Clear() { mLights.Clear(); for (uint i = 0; i < 32; ++i) mLayers[i].Clear(); }

	// Adds a new light to the draw queue
	void Add (Light* light) { mLights.Expand() = light; }

	// Add a new object to the draw queue. The 'group' parameter can be a material,
	// a texture, or anything else you might want to group similar objects by.
	void Add (uint layer, Object* obj, uint mask, uint group, float distSquared)
	{
		mLayers[layer & 31].Add(obj, mask, group, distSquared);
	}

	// Sort all objects by group and distance to camera
	void Sort() { for (uint i = 0; i < 32; ++i) mLayers[i].Sort(); }

	// Draw the scene
	uint Draw (const Deferred::Storage& storage, IGraphics* graphics, const Techniques& techniques);
};