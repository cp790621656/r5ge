#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the label's alignment
//============================================================================================================

void UILabel::SetAlignment (char alignment)
{
	if (mAlignment != alignment)
	{
		mAlignment = alignment;
		SetDirty();
	}
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UILabel::OnFill (UIQueue* queue)
{
	if (queue->mLayer != mLayer || queue->mArea != 0) return;

	const ITexture* tex ( GetTexture() );
	byte height ( GetFontSize() );

	if (tex != 0 && tex == queue->mTex)
	{
		uint width = Float::RoundToUInt(mRegion.GetWidth());

		Color4ub color ( mColor, mRegion.GetAlpha() );
		Vector2f pos   ( mRegion.GetLeft(), mRegion.GetTop() );

		if ( mAlignment == Alignment::Right )
		{
			mEnd = mText.GetLength();
			mStart = mEnd - mFont->CountChars( mText, width, 0, 0xFFFFFFFF, true, false, mTags );

			// Position needs to be adjusted by the difference between label's width and the length of the text
			pos.x += width - mFont->GetLength( mText, mStart, mEnd, mTags );
		}
		else
		{
			mStart = 0;
			mEnd = mFont->CountChars( mText, Float::RoundToUInt(mRegion.GetWidth()),
				0, 0xFFFFFFFF, false, false, mTags );

			// For centered alignment simply figure out the bounding size of the text and adjust the position
			if ( mAlignment == Alignment::Center )
			{
				uint size = mFont->GetLength( mText, mStart, mEnd, mTags );
				if (size < width) pos.x += (width - size) / 2;
			}
		}

		// Adjust the height in order to center the text as necessary
		float difference = mRegion.GetHeight() - height;
		pos.y += difference * 0.5f;

		//pos.x = Float::Floor(pos.x + 0.5f);
		//pos.y = Float::Floor(pos.y + 0.5f);

		// Drop a shadow if requested
		if (mShadow)
		{
			mFont->Print( queue->mVertices, pos + 1.0f, GetShadowColor(), mText, mStart, mEnd,
				(mTags == IFont::Tags::Ignore) ? IFont::Tags::Ignore : IFont::Tags::Skip );
		}

		// Print the text directly into the buffer
		mFont->Print( queue->mVertices, pos, color, mText, mStart, mEnd, mTags );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UILabel::CustomSerializeFrom(const TreeNode& root)
{
	if (UITextLine::CustomSerializeFrom(root))
	{
		return true;
	}
	else if (root.mTag == "Alignment")
	{
		String alignment;

		if (root.mValue >> alignment)
		{
			if		(alignment == "Left")	SetAlignment(Alignment::Left);
			else if (alignment == "Right")	SetAlignment(Alignment::Right);
			else							SetAlignment(Alignment::Center);
		}
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UILabel::CustomSerializeTo(TreeNode& root) const
{
	UITextLine::CustomSerializeTo(root);

	if		(mAlignment == Alignment::Left)		root.AddChild("Alignment", "Left");
	else if (mAlignment == Alignment::Right)	root.AddChild("Alignment", "Right");
	else										root.AddChild("Alignment", "Center");
}