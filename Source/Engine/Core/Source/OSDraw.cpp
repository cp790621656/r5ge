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
	if (mCore == 0) mCore = mObject->GetCore();
	if (mCore != 0) mCore->RemoveOnDraw( bind(&OSDraw::OnDraw, this) );
	mScene.Release();
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSDraw::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Shadows", mShadows);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSDraw::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Shadows") node.mValue >> mShadows;
}