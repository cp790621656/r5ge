#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Subdivisioned child of the Terrain class
// Author: Michael Lyashenko
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
	virtual void OnDraw (uint group, const ITechnique* tech, bool insideOut);
};