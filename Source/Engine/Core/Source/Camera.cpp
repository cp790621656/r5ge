#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Sets the camera's range based on the specified absolute values
//============================================================================================================

void Camera::SetAbsoluteRange (const Vector3f& range)
{
	mRelativeRange.x = range.x / mAbsoluteScale;
	mRelativeRange.y = range.y / mAbsoluteScale;
	mRelativeRange.y = range.z;
}

//============================================================================================================
// Calculates the absolute range based on the relative range and absolute scale
//============================================================================================================

void Camera::OnUpdate()
{
	mAbsoluteRange.x = mRelativeRange.x * mAbsoluteScale;
	mAbsoluteRange.y = mRelativeRange.y * mAbsoluteScale;
	mAbsoluteRange.z = mRelativeRange.z;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Camera::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Range", mRelativeRange);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Camera::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Range") node.mValue >> mRelativeRange;
	else return false;
	return true;
}