#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Initial validation of the draw script
//============================================================================================================

void OSDraw::OnInit()
{
	// Find the root
	mRoot = OSSceneRoot::FindRootOf(mObject);

	// For convenience
	mCore		= mObject->GetCore();
	mGraphics	= mObject->GetGraphics();

	// Set the root of the scene
	mScene.SetRoot(mRoot->GetOwner());

	// Initialize the shadow class
	mShadow.Initialize(mGraphics);
	
	// Add the draw callback to a high priority so that it's drawn before the default (1000) callbacks
	mCore->AddOnDraw( bind(&OSDraw::OnDraw, this), 10000 );
}

//============================================================================================================
// Remove the draw callback from the core
//============================================================================================================

void OSDraw::OnDestroy()
{
	mCore->RemoveOnDraw( bind(&OSDraw::OnDraw, this) );
	mShadow.Release();
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSDraw::OnSerializeTo (TreeNode& node) const
{
	if (mGrid) node.AddChild("Grid", mGrid);
	mShadow.OnSerializeTo(node.AddChild("Shadowmap"));
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSDraw::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Grid")
	{
		node.mValue >> mGrid;
	}
	else if (node.mTag == "Shadowmap")
	{
		FOREACH(i, node.mChildren)
		{
			mShadow.OnSerializeFrom(node.mChildren[i]);
		}
	}
}