#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void BasicLabel::OnFill (UIQueue* queue)
{
	if (queue->mLayer	== mLayer &&
		queue->mTex		!= 0 &&
		queue->mTex		== GetTexture() &&
		queue->mArea	== 0)
	{
		byte height ( GetFontSize() );

		Color4ub color ( mColor, mRegion.GetCalculatedAlpha() );
		Vector2f pos   ( mRegion.GetCalculatedLeft(), mRegion.GetCalculatedTop() );

		mEnd = mStart + mFont->CountChars( mText, Float::RoundToUInt(mRegion.GetCalculatedWidth()),
			mStart, 0xFFFFFFFF, false, false, mTags );

		// Adjust the height in order to center the text as necessary
		float difference = mRegion.GetCalculatedHeight() - height;
		pos.y += difference * 0.5f;

		// Drop a shadow if requested
		if (mShadow)
		{
			mFont->Print( queue->mVertices, pos + 1.0f, GetShadowColor(), mText, mStart, mEnd,
				(mTags == IFont::Tags::Ignore) ? IFont::Tags::Ignore : IFont::Tags::Skip );
		}
		mFont->Print( queue->mVertices, pos, color, mText, mStart, mEnd, mTags );
	}
}