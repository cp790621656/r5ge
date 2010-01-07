#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Subdivisioned child of the Terrain class
//============================================================================================================

class TerrainNode : public QuadNode
{
protected:

	IVBO*	mVBO;
	IVBO*	mIBO;
	uint	mIndices;

private:

	// Only the Terrain class can create new TerrainNodes
	friend class Terrain;

	TerrainNode() : mVBO(0), mIBO(0), mIndices(0) {}

public:

	// Should create the node's topology and update 'mBounds'
	virtual void OnFill (void* ptr, float bboxPadding);

	// Draw the object using the specified technique
	virtual void OnDraw (const ITechnique* tech, bool insideOut);
};