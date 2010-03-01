#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Subdivisioned child of the QuadTree hierarchy
//============================================================================================================

class QuadTree;
class QuadNode
{
public:

	friend class QuadTree;

	// Function delegate that will create the new nodes
	typedef FastDelegate<QuadNode* (void)> OnCreateCallback;

protected:

	QuadTree*	mTree;		// Pointer back to the tree that owns this node
	QuadNode*	mPart[4];	// Each QuadNode is always made up of up to 4 subdivisions
	Bounds		mBounds;	// Bounding volume for this node
	Vector2f	mOffset;	// Relative XY offset of this node (0 to <1 range)
	Vector2f	mSize;		// Relative size of this node (0 to 1 range)
	uint		mLevel;		// Node's level in the hierarchy, with 0 being the most detailed
	bool		mLeaf;		// Whether this node is a final leaf (has no children)

	// Child objects that belong to this node
	Array<Object*>	mChildren;

public:

	QuadNode();
	virtual ~QuadNode();

private:

	// Removes the specified object from the hierarchy, return 'true' if the object has been removed
	bool _Remove (Object* obj);

	// Adds the specified object to the hierarchy, returning 'true' if the object has been added
	bool _Add (Object* obj);
	bool _Add (Object* obj, const Bounds& bounds);

	// Subdivides the node further
	void _Partition (const OnCreateCallback& onCreate, float horizontal, float vertical);

	// Calls 'OnFill' on appropriate nodes
	void _FillGeometry (void* ptr, float bboxPadding);

	// Called when the object is being considered for rendering
	void Fill (Array<QuadNode*>& renderList, FillParams& params);

protected:

	// Should create the node's topology and update 'mBounds'
	virtual void OnFill (void* ptr, float bboxPadding)=0;

	// Draw the object using the specified technique
	virtual void OnDraw (const ITechnique* tech, bool insideOut)=0;
};