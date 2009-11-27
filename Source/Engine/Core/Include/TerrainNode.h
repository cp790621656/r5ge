#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
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

public:

	TerrainNode() : mVBO(0), mIBO(0), mIndices(0) {}

	// Should create the node's topology and update 'mBounds'
	virtual void OnFill (void* ptr);

	// Draw the object using the specified technique
	virtual uint OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut);
};