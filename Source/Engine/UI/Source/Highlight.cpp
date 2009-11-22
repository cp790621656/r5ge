#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void Highlight::OnFill (Queue* queue)
{
	if (queue->mTex		== 0 &&
		queue->mLayer	== mLayer &&
		queue->mArea	== 0)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetLeft()	);
		float top	 ( mRegion.GetTop()		);
		float right	 ( mRegion.GetRight()	);
		float bottom ( mRegion.GetBottom()	);

		Color4ub upper ( mColor, mRegion.GetAlpha() );
		Color4ub lower ( upper.r, upper.g, upper.b, Float::ToRangeByte(mRegion.GetAlpha()) );

		v.Expand().Set( left,  top,		0.0f, 0.0f, upper );
		v.Expand().Set( left,  bottom,	0.0f, 1.0f, lower );
		v.Expand().Set( right, bottom,	1.0f, 1.0f, lower );
		v.Expand().Set( right, top,		1.0f, 0.0f, upper );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool Highlight::CustomSerializeFrom(const TreeNode& root)
{
	if (root.mTag == "Color")
	{
		Color4f color4 (1.0f);

		if (root.mValue >> color4)
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

void Highlight::CustomSerializeTo(TreeNode& root) const
{
	root.AddChild("Color", mColor);
}