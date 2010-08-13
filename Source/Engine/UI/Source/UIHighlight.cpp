#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIHighlight::OnFill (UIQueue* queue)
{
	if (queue->mTex		== 0 &&
		queue->mLayer	== mLayer &&
		queue->mWidget	== 0)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetCalculatedLeft()	);
		float top	 ( mRegion.GetCalculatedTop()	);
		float right	 ( mRegion.GetCalculatedRight()	);
		float bottom ( mRegion.GetCalculatedBottom());

		Color4ub upper ( mTopColor, mRegion.GetCalculatedAlpha() );
		Color4ub lower ( mBottomColor, mRegion.GetCalculatedAlpha() );

		v.Expand().Set( left,  top,		0.0f, 0.0f, upper );
		v.Expand().Set( left,  bottom,	0.0f, 1.0f, lower );
		v.Expand().Set( right, bottom,	1.0f, 1.0f, lower );
		v.Expand().Set( right, top,		1.0f, 0.0f, upper );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIHighlight::OnSerializeFrom (const TreeNode& node)
{
	Color4ub color (0xFFFFFFFF);

	// Legacy support
	if (node.mTag == "Color")
	{
		if (node.mValue >> color)
		{
			SetTopColor(color);
			SetBottomColor(color);
		}
		return true;
	}
	else if (node.mTag == "Top Color")
	{
		if (node.mValue >> color) SetTopColor(color);
		return true;
	}
	else if (node.mTag == "Bottom Color")
	{
		if (node.mValue >> color) SetBottomColor(color);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIHighlight::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Top Color", mTopColor);
	node.AddChild("Bottom Color", mBottomColor);
}