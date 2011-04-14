#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Quad-tree split scene object, can be extended to create terrains
// Author: Michael Lyashenko
//============================================================================================================

class QuadTree : public Object
{
public:

	typedef QuadNode::OnCreateCallback OnCreateCallback;

protected:

	QuadNode*			mRootNode;		// Root of the tree
	Array<QuadNode*>	mRenderable;	// Temporary list of renderable nodes
	uint				mMaxNodes;

public:

	QuadTree();
	virtual ~QuadTree() { if (mRootNode != 0) delete mRootNode; }

	// QuadTree is an abstract class, so mark it as such
	R5_DECLARE_ABSTRACT_CLASS("QuadTree", Object);

	// Changes the default drawing layer that will be used by all QuadTrees
	static void SetDefaultLayer(byte layer);

	// Percentage of how much of the terrain is currently visible
	float GetVisibility() const { return mRenderable.GetSize() / (float)mMaxNodes; }

	// Splits the QuadTree down until the subdivisions reach the specified desired dimensions
	void PartitionToSize (uint desiredX, uint desiredY, uint currentX, uint currentY);

	// Split the tree into the specified number of parts
	void PartitionInto (uint horizontal, uint vertical);

	// Fill the terrain's topology after partitioning the QuadTree.
	// Height padding extends the height of the bounding box by this amount so child objects can fit easier.
	void FillGeometry (void* ptr, float bboxPadding = 0.0f);

protected:

	// Derived classes must override this function
	virtual QuadNode* _CreateNode()=0;

	// Should return a unique identifier used by the draw queue
	virtual uint GetUID() const=0;

	// Should retrieve the technique mask that the terrain can be rendered with (should not include children)
	virtual uint GetMask() const=0;

	// Function called when a new child object has been added
	virtual void OnAddChild (Object* obj) { mRootNode->_Add(obj); }

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj) { mRootNode->_Remove(obj); }

	// QuadTree often needs to reposition the child objects in its hierarchy
	virtual void OnPostUpdate();

	// Called when the object is being culled
	virtual bool OnFill (FillParams& params);

	// Run through all renderable nodes and draw them
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);
};