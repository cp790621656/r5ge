#include "../Include/_All.h"
using namespace R5;

//==============================================================================================
// Calculates the 4 texture coordinates for the image of specified size
//==============================================================================================

Rectangle<float> UIFace::GetRectangle (const Vector2i& size) const
{
	// NOTE: The face coordinates are stored as top-left based, matching Photoshop and
	// windows coordinates. R5 textures are bottom-left based, however. Solution? Flip the Y.
	Rectangle<float> out;
	out.left	=  ((float)mPos.x) / size.x;
	out.top		= -((float)mPos.y) / size.y;
	out.bottom	= -((float)(mPos.y + mSize.y)) / size.y;
	out.right	=  ((float)(mPos.x + mSize.x)) / size.x;
	return out;
}

//==============================================================================================
// Serialization -- Load
//==============================================================================================

void UIFace::SerializeFrom (const TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if		(tag == "Position") value >> mPos;
		else if (tag == "Size")		value >> mSize;
		else if (tag == "Border")
		{
			int val;

			if (value >> val)
			{
				mBorder = (short)val;
			}
		}
	}
}

//==============================================================================================
// Serialization -- Save
//==============================================================================================

void UIFace::SerializeTo (TreeNode& root) const
{
	if (mSize != 0)
	{
		TreeNode& node = root.AddChild("Face", mName);

		node.AddChild("Position", mPos);
		node.AddChild("Size", mSize);

		if (mBorder != 0) node.AddChild("Border", (int)mBorder);
	}
}