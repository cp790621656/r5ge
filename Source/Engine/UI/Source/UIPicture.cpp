#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the texture for the image
//============================================================================================================

void UIPicture::SetTexture (const ITexture* tex)
{
	if (mTex != tex)
	{
		if (mTex) OnDirty(mTex);
		mTex = tex;
		if (mTex) OnDirty(mTex);
	}
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIPicture::OnFill (UIQueue* queue)
{
	if (mTex != 0 &&
		queue->mLayer	== mLayer &&
		queue->mTex		== mTex &&
		queue->mWidget	== 0)
	{
		queue->mIgnoreAlpha = mIgnoreAlpha;

		float left	 ( mRegion.GetCalculatedLeft() );
		float top	 ( mRegion.GetCalculatedTop() );
		float right	 ( mRegion.GetCalculatedRight() );
		float bottom ( mRegion.GetCalculatedBottom() );

		Color4ub color (mColor, mRegion.GetCalculatedAlpha());

		float u = 1.0f, v = 1.0f;

		if (mTiled)
		{
			u = mRegion.GetCalculatedWidth()  / mTex->GetSize().x;
			v = mRegion.GetCalculatedHeight() / mTex->GetSize().y;
		}

		queue->mVertices.Expand().Set( left,  top,		0.0f, v, color );
		queue->mVertices.Expand().Set( left,  bottom,	0.0f, 0.0f, color );
		queue->mVertices.Expand().Set( right, bottom,	u, 0.0f, color );
		queue->mVertices.Expand().Set( right, top,		u, v, color );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIPicture::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;

	if (node.mTag == "Tiled")
	{
		bool val;
		if (value >> val) SetTiled(val);
		return true;
	}
	else if (node.mTag == "Back Color")
	{
		Color4ub c;
		if (value >> c) SetBackColor(c);
		return true;
	}
	else if (node.mTag == "Texture")
	{
		SetTexture ( mUI->GetTexture( value.AsString() ) );
		return true;
	}
	else if (node.mTag == "Ignore Alpha")
	{
		value >> mIgnoreAlpha;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIPicture::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Tiled", mTiled);
	node.AddChild("Back Color", mColor);
	if (mTex != 0) node.AddChild("Texture", mTex->GetName());
	node.AddChild("Ignore Alpha", mIgnoreAlpha);
}