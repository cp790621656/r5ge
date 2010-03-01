#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Counts the number of partitions necessary to achieve the desired dimension
//============================================================================================================

uint CountPartitions (uint desired, uint starting)
{
	uint part = 1;
	for ( ; starting > desired; starting >>= 1 ) part <<= 1;
	return part;
}

//============================================================================================================
// Changes the default drawing layer that will be used by the QuadTree
//============================================================================================================

byte g_quadLayer = 0;

void QuadTree::SetDefaultLayer (byte layer)
{
	g_quadLayer = layer & 31;
}

//============================================================================================================
// Assume the default layer when first created
//============================================================================================================

QuadTree::QuadTree() : mRootNode(0)
{
	mLayer = g_quadLayer;
}

//============================================================================================================
// Splits the QuadTree down until the subdivisions reach the specified desired dimensions
//============================================================================================================

void QuadTree::PartitionToSize (uint desiredX, uint desiredY, uint currentX, uint currentY)
{
	PartitionInto( CountPartitions(desiredX, currentX), CountPartitions(desiredY, currentY) );
}

//============================================================================================================
// Split the tree into the specified number of parts
//============================================================================================================

void QuadTree::PartitionInto (uint horizontal, uint vertical)
{
	if (mRootNode != 0) delete mRootNode;

	mRootNode = _CreateNode();
	mRootNode->mTree = this;
	mRootNode->mSize.Set(1.0f, 1.0f);
	mRootNode->mOffset.Set(0.0f, 0.0f);
	mRootNode->mLevel = 0;

	if (horizontal == 0) horizontal = 1;
	if (vertical   == 0) vertical   = 1;

	mMaxNodes = horizontal * vertical;

	mRootNode->_Partition(bind(&QuadTree::_CreateNode, this), 1.0f / horizontal, 1.0f / vertical);
}

//============================================================================================================
// Fills the bottom-most layer of the QuadTree (subdivisions with no children)
//============================================================================================================

void QuadTree::FillGeometry (void* ptr, float bboxPadding)
{
	if (mRootNode != 0 && ptr != 0)
	{
		mRootNode->_FillGeometry(ptr, bboxPadding);
	}
}

//============================================================================================================
// Objects that moved need to be re-added to a potentially different location in the tree
//============================================================================================================

void QuadTree::OnPostUpdate()
{
	for (uint i = mChildren.GetSize(); i > 0; )
	{
		Object* child = mChildren[--i];

		if (child->IsDirty())
		{
			mRootNode->_Remove(child);
			mRootNode->_Add(child);
		}
	}
}

//============================================================================================================
// Called when the object is being culled -- should return whether the object is visible
//============================================================================================================

bool QuadTree::OnFill (FillParams& params)
{
	// Clear the list of renderable objects
	mRenderable.Clear();

	// Cull the hierarchy, filling the list with renderable objects
	if (mRootNode != 0)
	{
		mRootNode->Fill(mRenderable, params);

		if (mRenderable.IsValid())
		{
			params.mDrawQueue.Add(mLayer, this, GetMask(), GetUID(), 0.0f);
		}
	}
	// Don't cull the children as they should already be culled by the subdivided nodes
	return false;
}

//============================================================================================================
// Run through all renderable nodes and draw them
//============================================================================================================

uint QuadTree::OnDraw (uint group, const ITechnique* tech, bool insideOut)
{
	for (uint i = 0, imax = mRenderable.GetSize(); i < imax; ++i)
	{
		mRenderable[i]->OnDraw(group, tech, insideOut);
	}
	return mRenderable.GetSize();
}