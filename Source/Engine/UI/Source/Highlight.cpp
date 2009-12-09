#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIHighlight::OnFill (UIQueue* queue)
{
	if (queue->mTex		== 0 &&
		queue->mLayer	== mLayer &&
		queue->mArea	== 0)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetCalculatedLeft()	);
		float top	 ( mRegion.GetCalculatedTop()		);
		float right	 ( mRegion.GetCalculatedRight()	);
		float bottom ( mRegion.GetCalculatedBottom()	);

		Color4ub upper ( mColor, mRegion.GetCalculatedAlpha() );
		Color4ub lower ( upper.r, upper.g, upper.b, Float::ToRangeByte(mRegion.GetCalculatedAlpha()) );

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
	if (node.mTag == "Color")
	{
		Color4f color4 (1.0f);

		if (node.mValue >> color4)
		{
			SetColor(color4);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIHighlight::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Color", mColor);
}