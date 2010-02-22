#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Octree-partitioned space
//============================================================================================================

class Octree : public Object
{
protected:

	// The QuadTree is made up of series of subdivided nodes
	struct Node : public Object
	{
		Octree*			mOctree;		// Octree this node belongs to
		uint			mDepth;			// Node's depth in the hierarchy
		Bounds			mBounds;		// This node's bounds
		Array<Node>		mPart;			// Subdivided child nodes
		Array<Object*>	mChildren;		// Personal list of objects residing in this node

		Node() : mOctree(0), mDepth(0) {}

		// Partitions the node, subdividing it into smaller nodes as necessary
		void Partition (Octree* tree, float x, float y, float z, const Vector3f& size, uint currentDepth, uint targetDepth);

		// Adds the specified object to the hierarchy, returning 'true' if the object has been added
		bool Add (Object* obj);
		bool Add (Object* obj, const Bounds& bounds);
		bool Add (Object* obj, const Vector3f& pos);

		// Fill the draw list
		void Fill (FillParams& params);
	};

	// Let the Node class access 'mCustomFill' for simplicity
	friend struct Octree::Node;

protected:

	Node		mRootNode;		// The tree always has one root node
	Vector3f	mSize;			// Size of the tree (extends from the center)
	uint		mDepth;			// Depth of node subdivision
	bool		mPartitioned;	// Whether the tree has been partitioned
	bool		mCustomFill;	// Whether the virtual OnFill function will be called

public:

	Octree() : mDepth(0), mPartitioned(true), mCustomFill(true) { mRootNode.mOctree = this; }

	R5_DECLARE_INHERITED_CLASS("Octree", Octree, Object, Object);

protected:

	// Repartitions the Octree then re-adds all children to sub-nodes
	void Repartition();

	// Function called when a new child object has been added
	virtual void OnAddChild (Object* obj);

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj);

	// QuadTree often needs to reposition the child objects in its hierarchy
	virtual void OnPostUpdate();

	// Called when the object is being culled
	virtual bool OnFill (FillParams& params);

	// Overwrite this function with your own custom behavior if additional checks are required
	// that may affect the decision of whether the object should be visible or not.
	// NOTE: Don't set 'mCustomFill' to 'false' if you overwrite this function!
	virtual void OnFillObject (Object* obj, FillParams& params) { obj->Fill(params); mCustomFill = false; }

	// Called when the object is being saved
	virtual void OnSerializeTo (TreeNode& node) const;

	// Called when the object is being loaded
	virtual bool OnSerializeFrom (const TreeNode& node);
};