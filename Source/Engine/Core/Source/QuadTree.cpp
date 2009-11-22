#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// QuadTree can be created by the scene manager (empty constructor) as well as itself (overloaded constructor)
//============================================================================================================

/*QuadTree::QuadTree() : mTop(0), mOffset(0.0f, 0.0f), mSize(1.0f, 1.0f), mLevel(-1)
{
	mPart[0] = 0;
	mPart[1] = 0;
	mPart[2] = 0;
	mPart[3] = 0;
}

//============================================================================================================
// QuadTree's destructor has to release all of the partitions
//============================================================================================================

QuadTree::~QuadTree()
{
	// Ensure to delete all partitions
	for (uint i = 0; i < 4; ++i)
		if (mPart[i] != 0) delete mPart[i];

	// Unless this is a top level node, skip the Object's destructor's Parent->RemoveChild
	// calls since this node has never been added as a child of anything.
	if (mTop != 0) mParent = 0;
}

//============================================================================================================
// INTERNAL: Used for tree subdivisioning
//============================================================================================================

void QuadTree::_Set (const String&	 name,
					 Core*			 core,
					 Scene*			 root,
					 Object*		 parent,
					 QuadTree*		 top,
					 const Vector2f& offset,
					 const Vector2f& size)
{
	mName	= name;
	mCore   = core;
	mScene  = root;
	mParent = parent;
	mTop	= top;
	mOffset = offset;
	mSize	= size;
}

//============================================================================================================
// INTERNAL: Release all resourced used by the tree
//============================================================================================================

void QuadTree::_Release()
{
	mLevel = -1;
	for (uint i = 0; i < 4; ++i)
	{
		if (mPart[i] != 0)
		{
			delete mPart[i];
			mPart[i] = 0;
		}
	}
	OnRelease();
}

//============================================================================================================
// INTERNAL: Figures out what level of detail the tree belongs to
//============================================================================================================

uint QuadTree::_GetLevel()
{
	if (mLevel == -1)
	{
		mLevel = 0;

		for (uint i = 0; i < 4; ++i)
		{
			if (mPart[i] != 0)
			{
				uint child = mPart[i]->_GetLevel() + 1;
				if (child > mLevel) mLevel = child;
			}
		}
	}
	return mLevel;
}

//============================================================================================================
// Helper function used in QuadTree::_Partition below
//============================================================================================================

void GetLeftRight (float& left, float& right, const float width, const float desired)
{
	if (width > desired)
	{
		left = desired;

		// Keep doubling the desired value while it's less than the tile's width
		for (; left < width; left *= 2.0f) {}

		left *= 0.5f;

		// Right hand side is whatever is remaining
		right = width - left;
		if (right < 0.00001f) right = 0.0f;
	}
	else
	{
		left = width;
		right = 0.0f;
	}
}

//============================================================================================================
// INTERNAL: Partitions the tree
//============================================================================================================
// 2 | 3
// --+--
// 0 | 1
//============================================================================================================

void QuadTree::_Partition (float horizontal, float vertical)
{
	// Only continue if the tree can be partitioned
	if (mSize.x > horizontal || mSize.y > vertical)
	{
		float w0, w1, h0, h1;

		GetLeftRight(w0, w1, mSize.x, horizontal);
		GetLeftRight(h0, h1, mSize.y, vertical);

		float x0 = mOffset.x;
		float x1 = mOffset.x + w0;
		float y0 = mOffset.y;
		float y1 = mOffset.y + h0;

		// Top-level parent is always the top tree node
		QuadTree* top = _GetTopLevel();

		// Bottom-left
		mPart[0] = OnNew();
		mPart[0]->_Set(mName + "_0", mCore, mScene, this, top, Vector2f(x0, y0), Vector2f(w0, h0));
		mPart[0]->_Partition(horizontal, vertical);

		if (w1 > 0.0f)
		{
			// Bottom-right
			mPart[1] = OnNew();
			mPart[1]->_Set(mName + "_1", mCore, mScene, this, top, Vector2f(x1, y0), Vector2f(w1, h0));
			mPart[1]->_Partition(horizontal, vertical);

			if (h1 > 0.0f)
			{
				// Top-right
				mPart[3] = OnNew();
				mPart[3]->_Set(mName + "_3", mCore, mScene, this, top, Vector2f(x1, y1), Vector2f(w1, h1));
				mPart[3]->_Partition(horizontal, vertical);
			}
		}

		if (h1 > 0.0f)
		{
			// Top-left
			mPart[2] = OnNew();
			mPart[2]->_Set(mName + "_2", mCore, mScene, this, top, Vector2f(x0, y1), Vector2f(w0, h1));
			mPart[2]->_Partition(horizontal, vertical);
		}
	}

	// Update the tree's level
	_GetLevel();
}

//============================================================================================================
// Count bottom-level tree partitions
//============================================================================================================

uint QuadTree::_CountPartitions (bool onlyVisible) const
{
	uint count ( mFlags.Get(Flag::Visible) ? 1 : 0 );

	if ( !onlyVisible || count != 0 )
	{
		for (uint i = 0; i < 4; ++i)
		{
			if (mPart[i] != 0)
			{
				count += mPart[i]->CountPartitions(onlyVisible);
			}
		}
	}
	return count;
}

//============================================================================================================
// Set the tree's topology via a heightmap
//============================================================================================================

void QuadTree::_CreateTree (void* ptr)
{
	// Reset the bounds
	mBounds.Reset();

	// Trigger the custom tree creation function
	OnCreate(ptr);

	// Fill the children partitions
	if (mPart[0] != 0) { mPart[0]->_CreateTree(ptr);	mBounds.Include(mPart[0]->GetBounds()); }
	if (mPart[1] != 0)	{ mPart[1]->_CreateTree(ptr);	mBounds.Include(mPart[1]->GetBounds()); }
	if (mPart[2] != 0)	{ mPart[2]->_CreateTree(ptr);	mBounds.Include(mPart[2]->GetBounds()); }
	if (mPart[3] != 0)	{ mPart[3]->_CreateTree(ptr);	mBounds.Include(mPart[3]->GetBounds()); }
}

//============================================================================================================
// QuadTree's culling function needs to pass an additional parameter
//============================================================================================================

void QuadTree::_CullTree (CullParams& params, bool isParentVisible, bool render, bool wasParentRendered)
{
	// By default this partition is not renderable
	mFlags.Set( Flag::Renderable, false );

	// Skip all the advanced rendering checks if the tree is not visible
	if ( isParentVisible &= params.mFrustum.IsVisible(mBounds) )
	{
		// The tree is renderable only if the parent hasn't been rendered
		if ( render && !wasParentRendered && OnCull(params) )
		{
			// 'OnCull' function added this object to the rendering queue
			mFlags.Set( Flag::Renderable, true );
			wasParentRendered = true;
		}
	}

	// Regardless of whether the tree is visible or not, fall through to children
	if (mPart[0] != 0) mPart[0]->_CullTree(params, isParentVisible, render, wasParentRendered);
	if (mPart[1] != 0) mPart[1]->_CullTree(params, isParentVisible, render, wasParentRendered);
	if (mPart[2] != 0) mPart[2]->_CullTree(params, isParentVisible, render, wasParentRendered);
	if (mPart[3] != 0) mPart[3]->_CullTree(params, isParentVisible, render, wasParentRendered);

	// Cull all attached objects
	mFlags.Set( Flag::Visible, isParentVisible );

	// Cull all children
	Object::_Cull( params, isParentVisible, render );
}

//============================================================================================================
// Partitions the tree into specified number of parts
//============================================================================================================

void QuadTree::Partition (uint horizontal, uint vertical)
{
	Lock();
	{
		if (horizontal == 0) horizontal = 1;
		if (vertical   == 0) vertical   = 1;

		_Release();
		_Partition(1.0f / horizontal, 1.0f / vertical);
	}
	Unlock();
}

//============================================================================================================
// Count bottom-level tree partitions
//============================================================================================================

uint QuadTree::CountPartitions (bool onlyVisible) const
{
	uint count (0);
	Lock();
	count = _CountPartitions(onlyVisible);
	Unlock();
	return count;
}

//============================================================================================================
// Object selection
//============================================================================================================

void QuadTree::OnSelect (const Vector3f& pos, ObjectPtr& ptr, float& radius)
{
	for (uint i = 0; i < 4; ++i)
	{
		QuadTree* child = mPart[i];
		
		if (child != 0 && child->GetFlag(Flag::Enabled))
		{
			child->OnSelect(pos, ptr, radius);
		}
	}
	Object::_Select(pos, ptr, radius);
}

//============================================================================================================
// Calls _PostUpdate() and recurses through children
//============================================================================================================

void QuadTree::_Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved)
{
	for (uint i = 0; i < 4; ++i)
	{
		QuadTree* child = mPart[i];
		
		if (child != 0 && child->GetFlag(Flag::Enabled))
		{
			child->_Update(pos, rot, scale, parentMoved);
		}
	}
	Object::_Update(pos, rot, scale, parentMoved);
}

//============================================================================================================
// Inherited from Object
//============================================================================================================

void QuadTree::_Cull (CullParams& params, bool isParentVisible, bool render)
{
	if (!mFlags.Get(Flag::Enabled)) return;

	Lock();
	{
		_CullTree(params, isParentVisible, render, false);
	}
	Unlock();
}

//============================================================================================================
// Render the object, called by the rendering queue this object was added to
//============================================================================================================
// NOTE: Note the usage of _GetTopLevel() and locking that level instead of current. That's because _Render()
// is the only function that can be called directly on a sub-tree, without going through the root.
//============================================================================================================

uint QuadTree::_Render (IGraphics* graphics, const ITechnique* tech, bool insideOut)
{
	uint count (0);
	QuadTree* top = _GetTopLevel();
	top->Lock();
	count += OnRender (graphics, tech, insideOut);
	top->Unlock();
	return count;
}*/