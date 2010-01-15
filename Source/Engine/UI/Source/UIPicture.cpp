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

		Color4ub color (255, 255, 255, Float::ToRangeByte( mRegion.GetCalculatedAlpha() ));

		queue->mVertices.Expand().Set( left,  top,		0.0f, 1.0f, color );
		queue->mVertices.Expand().Set( left,  bottom,	0.0f, 0.0f, color );
		queue->mVertices.Expand().Set( right, bottom,	1.0f, 0.0f, color );
		queue->mVertices.Expand().Set( right, top,		1.0f, 1.0f, color );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIPicture::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;

	if (node.mTag == "Texture")
	{
		SetTexture ( mUI->GetTexture( value.IsString() ? value.AsString() : value.GetString() ) );
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
	if (mTex != 0)
	{
		node.AddChild("Texture", mTex->GetName());
		node.AddChild("Ignore Alpha", mIgnoreAlpha);
	}
}