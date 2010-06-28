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

	// Add the draw callback to a high priority so that it's drawn before the default (1000) callbacks
	mCore->AddOnDraw( bind(&OSDraw::OnDraw, this), 10000 );
}

//============================================================================================================
// Remove the draw callback from the core
//============================================================================================================

void OSDraw::OnDestroy()
{
	mCore->RemoveOnDraw( bind(&OSDraw::OnDraw, this) );
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSDraw::OnSerializeTo (TreeNode& root) const
{
	if (mGrid) root.AddChild("Grid", mGrid);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSDraw::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Grid") node.mValue >> mGrid;
}