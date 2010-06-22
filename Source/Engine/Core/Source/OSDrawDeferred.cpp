#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Initialize the scene
//============================================================================================================

void OSDrawDeferred::OnInit()
{
	mCam = R5_CAST(Camera, mObject);

	if (mCam == 0)
	{
		ASSERT(false, "You've attached OSDrawDeferred to something other than a camera, unable to proceed!");
		DestroySelf();
	}
	else
	{
		OSDraw::OnInit();
	}
}

//============================================================================================================
// Draw callback
//============================================================================================================

void OSDrawDeferred::OnDraw()
{
	mScene.Cull(mCam);
	mScene.Draw(mBloom, mFocalRange, mSSAO);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSDrawDeferred::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Bloom", mBloom);
	root.AddChild("Focal Range", mFocalRange);
	root.AddChild("SSAO", mSSAO);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSDrawDeferred::OnSerializeFrom (const TreeNode& node)
{
	if		(node.mTag == "Bloom")		 node.mValue >> mBloom;
	else if (node.mTag == "Focal Range") node.mValue >> mFocalRange;
	else if (node.mTag == "SSAO")		 node.mValue >> mSSAO;
}