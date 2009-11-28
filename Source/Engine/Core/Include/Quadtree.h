#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Quad-tree split scene object, can be extended to create terrains
//============================================================================================================

class QuadTree : public Object
{
public:

	typedef QuadNode::OnCreateCallback OnCreateCallback;

	// QuadTree needs to position children
	struct Flag : Object::Flag
	{
		enum
		{
			HasMoved = 1 << 31,
		};
	};

protected:

	QuadNode*			mRootNode;		// Root of the tree
	Array<QuadNode*>	mRenderable;	// Temporary list of renderable nodes
	uint				mMaxNodes;

public:

	QuadTree() : mRootNode(0) {}
	virtual ~QuadTree() { if (mRootNode != 0) delete mRootNode; }

	// QuadTree is an abstract class, so mark it as such
	R5_DECLARE_ABSTRACT_CLASS("QuadTree", Object);

	// Percentage of how much of the terrain is currently visible
	float GetVisibility() const { return mRenderable.GetSize() / (float)mMaxNodes; }

	// Splits the quadtree down until the subdivisions reach the specified desired dimensions
	void PartitionToSize (uint desiredX, uint desiredY, uint currentX, uint currentY);

	// Split the tree into the specified number of parts
	void PartitionInto (uint horizontal, uint vertical);

	// Fill the terrain's topology after partitioning the QuadTree.
	// Height padding extends the height of the bounding box by this amount so child objects can fit easier.
	void Fill (void* ptr, float bboxPadding = 0.0f);

protected:

	// Derived classes must override this function
	virtual QuadNode* _CreateNode()=0;

	// QuadTree often needs to reposition the child objects in its hierarchy
	virtual void OnUpdate();
	virtual void OnPostUpdate();

	// Called when the object is being culled
	virtual CullResult OnCull (CullParams& params, bool isParentVisible, bool render);

	// Run through all renderable nodes and draw them
	virtual uint OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut);
};