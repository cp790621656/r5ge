#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the label's color
//============================================================================================================

void UITextLine::SetColor(const Color3f& color)
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
	if (mFont != 0)
	{
		const ITexture* tex = mFont->GetTexture();
		if (tex) OnDirty(tex);
	}
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UITextLine::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mTex != 0 && queue->mTex == GetTexture())
	{
		byte height = mFont->GetSize();

		Color4ub color ( mColor, mRegion.GetAlpha() );
		Vector2f pos   ( mRegion.GetLeft(), mRegion.GetTop() );

		// Adjust the height in order to center the text as necessary
		float difference = mRegion.GetHeight() - height;
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

bool UITextLine::CustomSerializeFrom(const TreeNode& root)
{
	const Variable& value = root.mValue;

	if (root.mTag == "Color")
	{
		Color3f color;
		if (value >> color) SetColor(color);
		return true;
	}
	else if (root.mTag == "Font")
	{
		const IFont* font = mRoot->GetFont( value.IsString() ? value.AsString() : value.GetString() );
		SetFont(font);
		return true;
	}
	else if (root.mTag == "Text")
	{
		SetText( value.IsString() ? value.AsString() : value.GetString() );
		return true;
	}
	else if (root.mTag == "Shadow")
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

void UITextLine::CustomSerializeTo(TreeNode& root) const
{
	root.AddChild("Color", mColor);

	TreeNode& font = root.AddChild("Font");
	if (mFont != 0) font.mValue = mFont->GetName();

	root.AddChild("Text", mText);
	root.AddChild("Shadow", mShadow);
}