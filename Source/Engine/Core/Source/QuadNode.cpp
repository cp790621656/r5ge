#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Helper function used in QuadNode::_Partition below
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
// All partitions need to start out empty
//============================================================================================================

QuadNode::QuadNode()
{
	mPart[0] = 0;
	mPart[1] = 0;
	mPart[2] = 0;
	mPart[3] = 0;
}

//============================================================================================================
// Release all of the partitions
//============================================================================================================

QuadNode::~QuadNode()
{
	for (uint i = 0; i < 4; ++i) if (mPart[i] != 0) delete mPart[i];
}

//============================================================================================================
// Removes the specified object from the hierarchy, return 'true' if the object has been removed
//============================================================================================================

bool QuadNode::_Remove (Object* obj)
{
	for (uint i = mChildren.GetSize(); i > 0; )
	{
		if (mChildren[--i] == obj)
		{
			mChildren.RemoveAt(i);
			return true;
		}
	}

	if (mPart[0] != 0 && mPart[0]->_Remove(obj)) return true;
	if (mPart[1] != 0 && mPart[1]->_Remove(obj)) return true;
	if (mPart[2] != 0 && mPart[2]->_Remove(obj)) return true;
	if (mPart[3] != 0 && mPart[3]->_Remove(obj)) return true;
	return false;
}

//============================================================================================================
// Adds the specified object to the hierarchy, returning 'true' if the object has been added
//============================================================================================================

bool QuadNode::_Add (Object* obj)
{
	// Directional lights affect everything so they should be added to the highest level possible
	if ( obj->IsOfClass(DirectionalLight::ClassID()) )
	{
		mChildren.Expand() = obj;
		return true;
	}

	// Point lights have a range we can use to calculate their bounds
	if ( obj->IsOfClass(PointLight::ClassID()) )
	{
		PointLight* light = (PointLight*)obj;
		Bounds bounds;
		bounds.Include(light->GetAbsolutePosition(), light->GetRange());
		return _Add(obj, bounds);
	}

	// If this is a model, then it already has bounds we can use
	if ( obj->IsOfClass(ModelInstance::ClassID()) )
	{
		ModelInstance* model = (ModelInstance*)obj;
		return _Add(obj, model->GetBounds());
	}

	// Otherwise simply use the world position
	return _Add(obj, obj->GetAbsolutePosition());
}

//============================================================================================================
// Adds the specified object to its proper place in the hierarchy using the specified bounds
//============================================================================================================

bool QuadNode::_Add (Object* obj, const Bounds& bounds)
{
	if ( mBounds.Contains(bounds) )
	{
		if (mPart[0] != 0 && mPart[0]->_Add(obj, bounds)) return true;
		if (mPart[1] != 0 && mPart[1]->_Add(obj, bounds)) return true;
		if (mPart[2] != 0 && mPart[2]->_Add(obj, bounds)) return true;
		if (mPart[3] != 0 && mPart[3]->_Add(obj, bounds)) return true;

		mChildren.Expand() = obj;
		return true;
	}
	else if (mLevel == 0)
	{
		mChildren.Expand() = obj;
		return true;
	}
	return false;
}

//============================================================================================================
// Adds the specified object to its proper place in the hierarchy using the specified position
//============================================================================================================

bool QuadNode::_Add (Object* obj, const Vector3f& pos)
{
	if (mBounds.Contains(pos))
	{
		if (mPart[0] != 0 && mPart[0]->_Add(obj, pos)) return true;
		if (mPart[1] != 0 && mPart[1]->_Add(obj, pos)) return true;
		if (mPart[2] != 0 && mPart[2]->_Add(obj, pos)) return true;
		if (mPart[3] != 0 && mPart[3]->_Add(obj, pos)) return true;

		mChildren.Expand() = obj;
		return true;
	}
	else if (mLevel == 0)
	{
		mChildren.Expand() = obj;
		return true;
	}
	return false;
}

//============================================================================================================
// INTERNAL: Partitions the node, subdividing it into smaller children
//============================================================================================================
// 2 | 3
// --+--
// 0 | 1
//============================================================================================================

void QuadNode::_Partition (const OnCreateCallback& onCreate, float horizontal, float vertical)
{
	// Release anything that may already be here
	if (mPart[0] != 0) { delete mPart[0]; mPart[0] = 0; }
	if (mPart[1] != 0) { delete mPart[1]; mPart[1] = 0; }
	if (mPart[2] != 0) { delete mPart[2]; mPart[2] = 0; }
	if (mPart[3] != 0) { delete mPart[3]; mPart[3] = 0; }

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

		// Bottom-left
		QuadNode* node = (mPart[0] = onCreate());
		node->mTree = mTree;
		node->mLevel = mLevel + 1;
		node->mSize.Set(w0, h0);
		node->mOffset.Set(x0, y0);
		node->_Partition(onCreate, horizontal, vertical);

		if (w1 > 0.0f)
		{
			// Bottom-right
			node = (mPart[1] = onCreate());
			node->mTree	= mTree;
			node->mLevel = mLevel + 1;
			node->mSize.Set(w1, h0);
			node->mOffset.Set(x1, y0);
			node->_Partition(onCreate, horizontal, vertical);

			if (h1 > 0.0f)
			{
				// Top-right
				node = (mPart[3] = onCreate());
				node->mTree	= mTree;
				node->mLevel = mLevel + 1;
				node->mSize.Set(w1, h1);
				node->mOffset.Set(x1, y1);
				node->_Partition(onCreate, horizontal, vertical);
			}
		}

		if (h1 > 0.0f)
		{
			// Top-left
			node = (mPart[2] = onCreate());
			node->mTree	= mTree;
			node->mLevel = mLevel + 1;
			node->mSize.Set(w0, h1);
			node->mOffset.Set(x0, y1);
			node->_Partition(onCreate, horizontal, vertical);
		}
	}
}

//============================================================================================================
// Calls 'OnFill' on appropriate nodes
//============================================================================================================

void QuadNode::_Fill (void* ptr, float padding)
{
	mBounds.Reset();

	mLeaf = true;

	if (mPart[0] != 0) { mPart[0]->_Fill(ptr, padding); mBounds.Include(mPart[0]->mBounds); mLeaf = false; }
	if (mPart[1] != 0) { mPart[1]->_Fill(ptr, padding); mBounds.Include(mPart[1]->mBounds); mLeaf = false; }
	if (mPart[2] != 0) { mPart[2]->_Fill(ptr, padding); mBounds.Include(mPart[2]->mBounds); mLeaf = false; }
	if (mPart[3] != 0) { mPart[3]->_Fill(ptr, padding); mBounds.Include(mPart[3]->mBounds); mLeaf = false; }

	// Only call 'OnFill' on the bottom-most node
	if (mLeaf) OnFill(ptr, padding);
}

//============================================================================================================
// Called when the object is being culled -- should return whether the object is visible
//============================================================================================================

void QuadNode::_Cull (Array<QuadNode*>& tiles, Object::CullParams& params, bool render)
{
	// Root level is always considered to be visible for the sake of object culling
	bool visible = (mLevel == 0);

	// If the node is visible, either render it or cull its subdivisions
	if (params.mFrustum.IsVisible(mBounds))
	{
		visible = true;

		if (mLeaf)
		{
			if (render)
			{
				tiles.Expand() = this;
			}
		}
		else
		{
			if (mPart[0] != 0) mPart[0]->_Cull(tiles, params, render);
			if (mPart[1] != 0) mPart[1]->_Cull(tiles, params, render);
			if (mPart[2] != 0) mPart[2]->_Cull(tiles, params, render);
			if (mPart[3] != 0) mPart[3]->_Cull(tiles, params, render);
		}
	}

	if (visible)
	{
		// Run through all children and cull them in turn
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			Object* obj = mChildren[i];
			if (obj != 0) obj->_Cull(params, true, render);
		}
	}
}