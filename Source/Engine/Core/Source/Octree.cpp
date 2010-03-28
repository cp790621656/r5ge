#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Partitions the node, subdividing it into smaller nodes as necessary
//============================================================================================================

void Octree::Node::Partition (Octree* tree, float x, float y, float z, const Vector3f& size, uint currentDepth, uint targetDepth)
{
	mOctree = tree;

	// Set this node's depth and bounds
	Vector3f pos (x, y, z);
	mDepth = currentDepth;
	mBounds.Set(pos - size, pos + size);

	// List of children must be clear at this point
	mChildren.Clear();

	// If we have not yet reached the desired depth, split this node into 8 more
	if (currentDepth++ < targetDepth)
	{
		Vector3f half (size.x * 0.5f, size.y * 0.5f, size.z * 0.5f);

		mPart.Clear();

		mPart.Expand().Partition(mOctree, x - half.x, y - half.y, z - half.y, half, currentDepth, targetDepth);
		mPart.Expand().Partition(mOctree, x + half.x, y - half.y, z - half.y, half, currentDepth, targetDepth);
		mPart.Expand().Partition(mOctree, x + half.x, y + half.y, z - half.y, half, currentDepth, targetDepth);
		mPart.Expand().Partition(mOctree, x - half.x, y + half.y, z - half.y, half, currentDepth, targetDepth);

		mPart.Expand().Partition(mOctree, x - half.x, y - half.y, z + half.z, half, currentDepth, targetDepth);
		mPart.Expand().Partition(mOctree, x + half.x, y - half.y, z + half.z, half, currentDepth, targetDepth);
		mPart.Expand().Partition(mOctree, x + half.x, y + half.y, z + half.z, half, currentDepth, targetDepth);
		mPart.Expand().Partition(mOctree, x - half.x, y + half.y, z + half.z, half, currentDepth, targetDepth);
	}
	else
	{
		mPart.Release();
	}
}

//============================================================================================================
// Adds the specified object to the hierarchy, returning 'true' if the object has been added
//============================================================================================================

bool Octree::Node::Add (Object* obj)
{
	const Bounds& bounds = obj->GetCompleteBounds();

	// If the bounds are invalid or we were unable to add this node to a child, add this object anyway
	if (!bounds.IsValid() || !Add(obj, bounds))
	{
		obj->SetSubParent(this);
		mChildren.Expand() = obj;
	}
	return true;
}

//============================================================================================================
// Adds the specified object to its proper place in the hierarchy using the specified bounds
//============================================================================================================

bool Octree::Node::Add (Object* obj, const Bounds& bounds)
{
	if ( mBounds.Contains(bounds) )
	{
		// Try to add to sub-nodes first
		for (uint i = mPart.GetSize(); i > 0; )
			if (mPart[--i].Add(obj, bounds))
				return true;

		// No sub-node claimed ownership: this node will own the object
		obj->SetSubParent(this);
		mChildren.Expand() = obj;
		return true;
	}
	return false;
}

//============================================================================================================
// Fill the draw list
//============================================================================================================

void Octree::Node::Fill (FillParams& params)
{
	if (mDepth == 0 || params.mFrustum.IsVisible(mBounds))
	{
		// Fill all sub-divisions
		for (uint i = mPart.GetSize(); i > 0; ) mPart[--i].Fill(params);

		// Run through all children and cull them in turn
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			Object* obj = mChildren[i];

			if (obj != 0)
			{
				if (mOctree->mCustomFill)
				{
					mOctree->OnFillObject(obj, params);
				}
				else
				{
					obj->Fill(params);
				}
			}
		}
	}
}

//============================================================================================================
// Raycast callback
//============================================================================================================

void Octree::Node::Raycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits)
{
	if (Intersect::RayBounds(pos, dir, mBounds))
	{
		// Run through all children and raycast them in turn
		for (uint i = mChildren.GetSize(); i > 0; ) mChildren[--i]->Raycast(pos, dir, hits);

		// Recurse through sub-divisions
		for (uint i = mPart.GetSize(); i > 0; ) mPart[--i].Raycast(pos, dir, hits);
	}
}

//============================================================================================================
// Octree's constructor: don't include child bounds as children already get added to proper nodes.
//============================================================================================================

Octree::Octree() : mDepth(0), mPartitioned(true), mCustomFill(true)
{
	mCalcAbsBounds		= false;
	mIncChildBounds		= false;
	mRootNode.mOctree	= this;
	mRootNode.SetName("Octree Root");
}

//============================================================================================================
// Repartitions the Octree then re-adds all children to sub-nodes
//============================================================================================================

void Octree::Repartition()
{
	mPartitioned = true;
	mRootNode.Partition(this, mAbsolutePos.x, mAbsolutePos.y, mAbsolutePos.z, mSize * mAbsoluteScale, 0, mDepth);
	for (uint i = mChildren.GetSize(); i > 0;) mRootNode.Add(mChildren[--i]);
}

//============================================================================================================
// Function called when a new child object has been added
//============================================================================================================

void Octree::OnAddChild (Object* obj)
{
	if (!mPartitioned) Repartition();
	else mRootNode.Add(obj);
}

//============================================================================================================
// Function called just before the child gets removed
//============================================================================================================

void Octree::OnRemoveChild (Object* obj)
{
	Node* sub = (Node*)obj->GetSubParent();

	// Remove the child from the node it currently belongs to
	if (sub != 0)
	{
		obj->SetSubParent(0);
		sub->mChildren.Remove(obj);
	}
}

//============================================================================================================
// Objects that moved need to be re-added to a potentially different location in the tree
//============================================================================================================

void Octree::OnPostUpdate()
{
	// Update partitioning if needed
	if (!mPartitioned) Repartition();

	// Run through all children and find ones that have moved
	for (uint i = mChildren.GetSize(); i > 0; )
	{
		Object* child = mChildren[--i];

		// If the child has moved, re-add it to the list (this places it into an appropriate node)
		if (child->HasMoved())
		{
			Node* sub = (Node*)child->GetSubParent();
			ASSERT(sub != 0, "How is sub-node set to null?");

			// Save time by re-adding the child to the same node.
			// This automatically recurses through the smaller child nodes.
			sub->mChildren.Remove(child);

			// If the child can't fit into its previous node, re-add it starting at the root.
			if (!sub->Add(child, child->GetCompleteBounds()))
			{
				mRootNode.Add(child);
			}
		}
	}

	// Absolute bounds should be the root node's bounds
	mAbsoluteBounds = mRootNode.mBounds;
	mRelativeBounds = mAbsoluteBounds;
	mCompleteBounds = mAbsoluteBounds;

	// Include all children that currently reside on the root node. Other children don't need to be included
	// as they belong to one of the internal nodes which is guaranteed to be inside the root's bounds.
	FOREACH(i, mRootNode.mChildren)
	{
		Object* child = mRootNode.mChildren[i];
		const Bounds& bounds = child->GetCompleteBounds();
		mCompleteBounds.Include(bounds);
	}
}

//============================================================================================================
// Called when the object is being culled
//============================================================================================================

bool Octree::OnFill (FillParams& params)
{
	// Fill the queues by recursing through children
	mRootNode.Fill(params);

	// Don't fill the children as they should already be filled by the subdivided nodes
	return false;
}

//============================================================================================================
// Cast a ray into the Octree
//============================================================================================================

bool Octree::OnRaycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits)
{
	// Raycast into the sub-divisions
	mRootNode.Raycast(pos, dir, hits);

	// Don't consider children as we've already considered them
	return false;
}

//============================================================================================================
// Called when the object is being saved
//============================================================================================================

void Octree::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Size", mSize);
	node.AddChild("Depth", mDepth);
}

//============================================================================================================
// Called when the object is being loaded
//============================================================================================================

bool Octree::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Size")
	{
		if (node.mValue.IsVector3f())
		{
			mSize = node.mValue.AsVector3f();
			mPartitioned = false;
		}
		return true;
	}
	else if (node.mTag == "Depth")
	{
		mDepth = node.mValue.AsUInt();
		mPartitioned = false;
		return true;
	}
	return false;
}