#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Quad-tree split scene object, can be extended to create terrains
//============================================================================================================

/*class QuadTree : public Object
{
public:

	struct Flag : public Object::Flag
	{
		enum
		{
			Visible		= 1 << 2,	// The object is inside the viewing volume
			Renderable	= 1 << 3,	// The object is inside the viewing volume and can be rendered
		};
	};

protected:

	QuadTree*	mTop;		// Top-level tree node
	QuadTree*	mPart[4];	// Subdivisions
	Bounds		mBounds;	// Bounding volume
	Vector2f	mOffset;	// Relative XY offset of this tree (0-1 range)
	Vector2f	mSize;		// Relative size of this tree (0-1 range)
	uint		mLevel;		// Tree's level in the hierarchy, with 0 being the most detailed

public:

	QuadTree();
	virtual ~QuadTree();

	// Object creation
	R5_DECLARE_ABSTRACT_CLASS("QuadTree", Object);

private:

	// INTERNAL: Used for tree subdivisioning
	void _Set (	const String&	 name,
				Core*			 core,
				Scene			 root,
				Object*			 parent,
				QuadTree*		 top,
				const Vector2f&  offset,
				const Vector2f&  size );

	// INTERNAL: Releases the memory, not thread safe
	void _Release();

	// INTERNAL: Figures out what level of detail the tree belongs to
	uint _GetLevel();

	// INTERNAL: Top-level parent is always the top tree node
	QuadTree* _GetTopLevel() { return (mTop == 0) ? this : mTop; }

	// INTERNAL: Split the tree into segments of specified width and height
	void _Partition (float horizontal, float vertical);

	// INTERNAL: Counts bottom-level tree partitions
	uint _CountPartitions (bool onlyVisible) const;

	// INTERNAL: Create the tree, calling OnCreate() from every partition
	void _CreateTree (void* ptr);

	// INTERNAL: Tree's culling function needs to pass an additional parameter
	void _CullTree (CullParams& params, bool isParentVisible, bool render, bool wasParentRendered);

public:

	// Releases the entire tree
	void Release() { Lock(); _Release(); Unlock(); }
	
	// The level on which this quadtree lies on
	uint GetLevel() const { return mLevel; }

	// Bounds of the quadtree node
	const Bounds&	GetBounds()	const { return mBounds; }

	// Counts the number of partitions necessary to achieve the desired dimension
	static uint CountPartitions (uint desired, uint starting)
	{
		uint part = 1;
		for ( ; starting > desired; starting >>= 1 ) part <<= 1;
		return part;
	}

	// Split the tree into the specified number of parts
	void Partition (uint horizontal, uint vertical);

	// Count bottom-level tree partitions
	uint CountPartitions (bool onlyVisible) const;

	// Create the tree, calling OnCreate from every partition
	void Create (void* ptr) { Lock(); _CreateTree(ptr); Unlock(); }

protected:

	// Object selection
	virtual void _Select (const Vector3f& pos, ObjectPtr& ptr, float& radius);

	// Calls _PostUpdate() and recurses through children
	virtual void _Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved);

	// Inherited from Object
	virtual void _Cull (CullParams& params, bool isParentVisible, bool render);

	// Inherited from Object
	virtual uint _Render (IGraphics* graphics, const ITechnique* tech, bool insideOut);

public:

	// MUST OVERRIDE: Creates a new tree of specified type -- used in partitioning: { return new YourType(); }
	virtual QuadTree* OnNew () const=0;

	// MUST OVERRIDE: Internal function that should release all allocated memory
	virtual void OnRelease()=0;

	// MUST OVERRIDE: Called when the tree is being filled -- should create all geometry here and update 'mBounds'
	virtual void OnCreate (void* ptr)=0;

	// MUST OVERRIDE: Called when the tree is being culled -- should add the object to the proper queue and return whether that happened
	virtual bool OnCull (CullParams& params)=0;

	// MUST OVERRIDE: Called when the tree is being rendered -- should render all geometry belonging to the specified group
	virtual uint OnRender (IGraphics* graphics, const ITechnique* tech, bool insideOut)=0;
};*/