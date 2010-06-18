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

	// Add a draw callback
	mCore->AddOnDraw(bind(&OSDraw::OnDraw, this));
}

//============================================================================================================
// Remove the draw callback from the core
//============================================================================================================

void OSDraw::OnDestroy()
{
	mCore->RemoveOnDraw(bind(&OSDraw::OnDraw, this));
}