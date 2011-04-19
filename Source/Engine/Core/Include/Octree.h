#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Octree-partitioned space
// Author: Michael Lyashenko
//============================================================================================================

class Octree : public Object
{
protected:

	// Abstract data member belonging to the Node class. Can be created and used by Octree-derived classes.
	struct Data
	{
		virtual ~Data() {}
	};

	// The QuadTree is made up of series of subdivided nodes
	struct Node
	{
		Octree*			mOctree;		// Octree this node belongs to
		uint			mDepth;			// Node's depth in the hierarchy
		Bounds			mBounds;		// This node's bounds
		Array<Node>		mPart;			// Subdivided child nodes
		Array<Object*>	mChildren;		// Personal list of objects residing in this node
		Data*			mData;			// Abstract data member, can be set and used by derived classes

		Node() : mOctree(0), mDepth(0), mData(0) {}
		~Node() { if (mData != 0) delete mData; }

		// Partitions the node, subdividing it into smaller nodes as necessary
		void Partition (Octree* tree, float x, float y, float z, const Vector3f& size, uint currentDepth, uint targetDepth);

		// Raycast callback
		void Raycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits);

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

	Array<Object*> mNewlyAdded;	// List of newly added objects that need to be placed into the tree

	// Objects should never be created manually. Use the AddObject<> template instead.
	Octree();

public:

	R5_DECLARE_INHERITED_CLASS("Octree", Octree, Object, Object);

protected:

	// Repartitions the Octree then re-adds all children to sub-nodes
	void Repartition();

	// Called when a newly added object gets positioned somewhere in the octree
	virtual void OnNewlyAdded (Object* obj) {}

	// Function called when a new child object has been added
	virtual void OnAddChild (Object* obj);

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj);

	// QuadTree often needs to reposition the child objects in its hierarchy
	virtual void OnPostUpdate();

	// Called when the object is being culled
	virtual bool OnFill (FillParams& params);

	// Cast a ray into the Octree
	virtual bool OnRaycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits);

	// Overwrite this function with your own custom behavior if additional checks are required
	// that may affect the decision of whether the objects should be visible or not.
	virtual void OnFillNode (Node& node, FillParams& params);

	// Called when the object is being saved
	virtual void OnSerializeTo (TreeNode& node) const;

	// Called when the object is being loaded
	virtual bool OnSerializeFrom (const TreeNode& node);
};