#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
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
	bool _Add (Object* obj, const Vector3f& pos);

	// Subdivides the node further
	void _Partition (const OnCreateCallback& onCreate, float horizontal, float vertical);

	// Calls 'OnCreate' on appropriate nodes
	void _Fill (void* ptr);

	// Called when the object is being culled, returns whether this node is visible
	void _Cull (Array<QuadNode*>& renderList, Object::CullParams& params, bool render);

	// Navigate down to the leaves and render them as necessary
	uint _Draw (IGraphics* graphics, const ITechnique* tech, bool insideOut);

protected:

	// Should create the node's topology and update 'mBounds'
	virtual void OnFill (void* ptr)=0;

	// Draw the object using the specified technique
	virtual uint OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut)=0;
};