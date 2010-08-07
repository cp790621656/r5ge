#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIBasicLabel::OnFill (UIQueue* queue)
{
	if (queue->mLayer	== mLayer &&
		queue->mTex		!= 0 &&
		queue->mTex		== GetTexture() &&
		queue->mWidget	== 0)
	{
		const IFont* font = GetFont();

		byte height ( GetFontSize() );

		Color4ub textColor ( mTextColor, mRegion.GetCalculatedAlpha() );
		Color4ub shadowColor ( mShadowColor, mRegion.GetCalculatedAlpha() );
		Vector2f pos ( mRegion.GetCalculatedLeft(), mRegion.GetCalculatedTop() );

		mEnd = mStart + font->CountChars( mText, Float::RoundToUInt(mRegion.GetCalculatedWidth()),
			mStart, 0xFFFFFFFF, false, false, mTags );

		// Adjust the height in order to center the text as necessary
		float difference = mRegion.GetCalculatedHeight() - height;
		pos.y += difference * 0.5f;

		// Drop a shadow if requested
		if (shadowColor.a != 0)
		{
			font->Print( queue->mVertices, pos + 1.0f, shadowColor, mText, mStart, mEnd,
				(mTags == IFont::Tags::Ignore) ? IFont::Tags::Ignore : IFont::Tags::Skip );
		}

		// Print the text directly into the buffer
		if (textColor.a != 0) font->Print( queue->mVertices, pos, textColor, mText, mStart, mEnd, mTags );
	}
}