#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// All drawable objects are separated by layers into different draw sets
//============================================================================================================

class DrawQueue
{
public:

	typedef Array<const ITechnique*> Techniques;

private:

	// Only the Scene class should be touching 'mLayers' directly
	friend class Scene;

	Light::List mLights;
	DrawLayer mLayers[32];

public:

	// Clear the draw queue
	inline void Clear() { mLights.Clear(); for (uint i = 0; i < 32; ++i) mLayers[i].Clear(); }

	// Adds a new light to the draw queue
	inline void Add (Light* light) { mLights.Expand() = light; }

	// Add a new object to the draw queue
	inline void Add (uint layer, Object* obj, uint mask, const void* group, float distSquared)
	{
		mLayers[layer & 31].Add(obj, mask, group, distSquared);
	}

	// Sort all objects by group and distance to camera
	inline void Sort() { for (uint i = 0; i < 32; ++i) mLayers[i].Sort(); }

	// Draw the scene
	uint Draw (IGraphics* graphics, const Techniques& techniques, bool insideOut);
};