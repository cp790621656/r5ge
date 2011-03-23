#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool SysWindow::SerializeFrom (const TreeNode& root)
{
	bool retVal (true);

	// If the window has already been created, only support "Size" and "Fullscreen" tags
	if (IsValid())
	{
		Vector2i size(0, 0);
		unsigned int style = Style::Undefined;

		for (unsigned int i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if ( tag == "Size" )
			{
				value >> size;
			}
			else if ( tag == "Full Screen" )
			{
				bool full;

				if (value >> full)
				{
					style = full ? Style::FullScreen : Style::Normal;
				}
			}
		}

        // Resize the window if necessary
		if (size != 0 || style != Style::Undefined)
		{
			retVal = SetSize(size) && SetStyle(style);
		}
	}
	else // If the window hasn't been created
	{
		String		title;
		Vector2i	size(1024, 768);
		Vector2i	pos(0, 0);
		bool		full (false);

		for (unsigned int i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if		( tag == "Name" || tag == "Title" )	value >> title;
			else if ( tag == "Size" )					value >> size;
			else if ( tag == "Position" )				value >> pos;
			else if ( tag == "Full Screen" )			value >> full;
		}

		// If there's at the very least a name present, create a window
		retVal = title.IsValid() ? Create( title, pos.x, pos.y, size.x, size.y, full ? Style::FullScreen : Style::Normal ) : false;
	}
	return retVal;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool SysWindow::SerializeTo (TreeNode& root) const
{
	if (mTitle.IsEmpty()) return false;

	TreeNode& node = root.AddChild("Window");
	node.AddChild("Title", mTitle);
	node.AddChild("Position", mPos);
	node.AddChild("Size", mSize);

	unsigned int style = (mStyle == Style::Undefined) ? mPrevStyle : mStyle;

	node.AddChild("Full Screen", style == Style::FullScreen);
	return true;
}