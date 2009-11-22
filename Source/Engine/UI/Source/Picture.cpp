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
		queue->mTex	== mTex &&
		queue->mArea	== 0)
	{
		queue->mIgnoreAlpha = mIgnoreAlpha;

		float left	 ( mRegion.GetLeft() );
		float top	 ( mRegion.GetTop() );
		float right	 ( mRegion.GetRight() );
		float bottom ( mRegion.GetBottom() );

		Color4ub color (255, 255, 255, Float::ToRangeByte( mRegion.GetAlpha() ));

		queue->mVertices.Expand().Set( left,  top,		0.0f, 1.0f, color );
		queue->mVertices.Expand().Set( left,  bottom,	0.0f, 0.0f, color );
		queue->mVertices.Expand().Set( right, bottom,	1.0f, 0.0f, color );
		queue->mVertices.Expand().Set( right, top,		1.0f, 1.0f, color );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIPicture::CustomSerializeFrom(const TreeNode& root)
{
	const Variable& value = root.mValue;

	if (root.mTag == "Texture")
	{
		SetTexture ( mRoot->GetTexture( value.IsString() ? value.AsString() : value.GetString() ) );
		return true;
	}
	else if (root.mTag == "Ignore Alpha")
	{
		value >> mIgnoreAlpha;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIPicture::CustomSerializeTo(TreeNode& root) const
{
	if (mTex != 0)
	{
		root.AddChild("Texture", mTex->GetName());
		root.AddChild("Ignore Alpha", mIgnoreAlpha);
	}
}