#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Rotate the owner
//============================================================================================================

void OSRotate::OnPreUpdate()
{
	Quaternion rot (mAxis, (float)(Time::GetSeconds() * (mRate * TWOPI)));
	mObject->SetRelativeRotation(rot);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSRotate::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Axis", mAxis);
	node.AddChild("Rate", mRate);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSRotate::OnSerializeFrom (const TreeNode& node)
{
	if		(node.mTag == "Axis") { node.mValue >> mAxis; mAxis.Normalize(); }
	else if (node.mTag == "Rate") node.mValue >> mRate;
}