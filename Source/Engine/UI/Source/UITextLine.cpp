#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves the font's associated texture
//============================================================================================================

const ITexture* UITextLine::GetTexture() const
{
	const IFont* font = GetFont();
	return (font != 0) ? font->GetTexture() : 0;
}

//============================================================================================================
// Retrieving the font should use the default if one hasn't been provided
//============================================================================================================

const IFont* UITextLine::GetFont() const
{
	if (mFont == 0 && mUI != 0) mFont = mUI->GetDefaultFont();
	return mFont;
}

//============================================================================================================
// Retrieves the size of the font
//============================================================================================================

byte UITextLine::GetFontSize() const
{
	const IFont* font = GetFont();
	return (font == 0) ? 0 : font->GetSize();
}

//============================================================================================================
// Changes the label's color
//============================================================================================================

void UITextLine::SetTextColor (const Color4ub& color)
{
	if (mColor != color)
	{
		mColor = color;
		SetDirty();
	}
}

//============================================================================================================
// Changes the shadow setting (text is drawn twice, once in black, once in normal color if it's on)
//============================================================================================================

void UITextLine::SetShadow (bool val)
{
	if (mShadow != val)
	{
		mShadow = val;
		SetDirty();
	}
}

//============================================================================================================
// Changes the label's text
//============================================================================================================

void UITextLine::SetText (const String& text)
{
	if (mText != text)
	{
		mText = text;
		if (GetAlpha() > 0.0f) SetDirty();
	}
}

//============================================================================================================
// Changes the font used by the label
//============================================================================================================

void UITextLine::SetFont (const IFont* font)
{
	if (mFont != font)
	{
		SetDirty();
		mFont = const_cast<IFont*>(font);
		SetDirty();
	}
}

//============================================================================================================
// Marks the associated queue as needing to be rebuilt
//============================================================================================================

void UITextLine::SetDirty()
{
	const ITexture* tex = GetTexture();
	if (tex != 0) OnDirty(tex);
}

//============================================================================================================
// Set the default font if one hasn't been chosen already
//============================================================================================================

bool UITextLine::OnUpdate (bool dimensionsChanged)
{
	if (mFont == 0) SetFont(mUI->GetDefaultFont());
	return false;
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UITextLine::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mTex != 0 && queue->mTex == GetTexture())
	{
		byte height = mFont->GetSize();

		Color4ub color ( mColor, mRegion.GetCalculatedAlpha() );
		Vector2f pos   ( mRegion.GetCalculatedLeft(), mRegion.GetCalculatedTop() );

		// Adjust the height in order to center the text as necessary
		float difference = mRegion.GetCalculatedHeight() - height;
		pos.y += difference * 0.5f;

		pos.x = Float::Round(pos.x);
		pos.y = Float::Round(pos.y);

		// Drop a shadow if requested
		if (mShadow)
		{
			mFont->Print( queue->mVertices, pos + 1.0f, GetShadowColor(), mText, 0, 0xFFFFFFFF,
				(mTags == IFont::Tags::Ignore) ? IFont::Tags::Ignore : IFont::Tags::Skip );
		}

		mFont->Print( queue->mVertices, pos, color, mText, 0, 0xFFFFFFFF, mTags );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UITextLine::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;

	if (node.mTag == "Text Color")
	{
		Color4ub color;
		if (value >> color) SetTextColor(color);
		return true;
	}
	else if (node.mTag == "Font")
	{
		const IFont* font = mUI->GetFont( value.AsString() );
		SetFont(font);
		return true;
	}
	else if (node.mTag == "Text")
	{
		SetText( value.AsString() );
		return true;
	}
	else if (node.mTag == "Shadow")
	{
		bool shadow;

		if (value >> shadow)
		{
			SetShadow(shadow);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UITextLine::OnSerializeTo (TreeNode& node) const
{
	if (mFont != 0 && mFont != mUI->GetDefaultFont())
		node.AddChild("Font", mFont->GetName());

	node.AddChild("Text Color", mColor);
	node.AddChild("Text", mText);
	node.AddChild("Shadow", mShadow);
}