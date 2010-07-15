#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Editable labels should intercept keyboard events
//============================================================================================================

UIEditableLabel::UIEditableLabel() :
	mHasFocus		(false),
	mSelectionEnd	(0),
	mSelectionStart	(0)
{
	mSelColor = Color4f(1.0f, 0.667f, 0.161f, 0.5f);
	mTags = IFont::Tags::Ignore;
	mEventHandling = EventHandling::Full;
}

//============================================================================================================
// Get the index of a character at the specified position
//============================================================================================================

uint UIEditableLabel::_GetIndexAt (const Vector2i& pos) const
{
	const IFont* font = GetFont();

	if ( font != 0 && mText.IsValid() && pos.x > mRegion.GetCalculatedLeft() )
	{
		uint width = Float::RoundToUInt(pos.x - mRegion.GetCalculatedLeft());
		uint charIndex = font->CountChars( mText, width, mStart, 0xFFFFFFFF, false, true, mTags );
		return mStart + charIndex;
	}
	return mStart;
}

//============================================================================================================
// Moves the cursor to the specified location
//============================================================================================================

void  UIEditableLabel::_MoveCursorTo(uint index)
{
	const IFont* font = GetFont();

	if (font != 0)
	{
		// 'index' can come in as 0xFFFFFFFF, so be sure to cap it at maximum length
		if (index > mText.GetLength())  index = mText.GetLength();

		bool shiftIsDown = mUI->IsKeyDown(Key::LeftShift) || mUI->IsKeyDown(Key::RightShift);
		bool selectionIsChanging = false;

		if (!shiftIsDown)
		{
			selectionIsChanging = (index != mSelectionStart);
			mSelectionStart	= index;
		}

		// The requested index is left of the visible start -- move the text
		if ( index < mStart )
		{
			mSelectionEnd	= index;
			mStart			= index;
			SetDirty();
		}
		// The requested index is right of the visible end -- move the text
		else if ( index > mEnd )
		{
			uint count = font->CountChars( mText,
				Float::RoundToUInt(mRegion.GetCalculatedWidth()), 0, index, true, false, mTags );

			mSelectionEnd	= index;
			mStart			= index - count;
			SetDirty();
		}
		// The requested index is on the visible text -- only change the highlight
		else if ( index != mSelectionEnd || selectionIsChanging )
		{
			mSelectionEnd = index;
			OnDirty(0, mLayer + 1);
		}
	}
}

//============================================================================================================
// Retrieves the text to the left and to the right of the selection
//============================================================================================================

void UIEditableLabel::_GetLeftRight (String& txtLeft, String& txtRight)
{
	uint left, right;

	if ( mSelectionEnd < mSelectionStart )
	{
		left  = mSelectionEnd;
		right = mSelectionStart;
	}
	else
	{
		left  = mSelectionStart;
		right = mSelectionEnd;
	}

	mText.GetString(txtLeft, 0, left);
	mText.GetString(txtRight, right);
}

//============================================================================================================
// Selects the entire text
//============================================================================================================

void UIEditableLabel::SelectAll()
{
	mSelectionStart = mText.GetLength();
	mSelectionEnd = 0;
	OnDirty(0, mLayer + 1);
}

//============================================================================================================
// Retrieves the selected text
//============================================================================================================

String  UIEditableLabel::GetSelectedText() const
{
	// String class automatically figures out which one is less than the other -- start or end
	String text;
	mText.GetString(text, mSelectionStart, mSelectionEnd);
	return text;
}

//============================================================================================================
// Changes the label's text
//============================================================================================================

void  UIEditableLabel::SetText (const String& text)
{
	if ( mText != text )
	{
		mText			= text;
		mStart			= 0;
		mSelectionEnd	= 0;
		mSelectionStart	= 0;

		if (GetAlpha() > 0.0f) SetDirty();
	}
}

//============================================================================================================
// Changes the label's font
//============================================================================================================

void  UIEditableLabel::SetFont (const IFont* font)
{
	if (mFont != font)
	{
		SetDirty();
		mFont			= const_cast<IFont*>(font);
		mStart			= 0;
		mSelectionEnd	= 0;
		mSelectionStart = 0;
		SetDirty();
	}
}

//============================================================================================================
// Changes the selection color
//============================================================================================================

void  UIEditableLabel::SetSelectionColor (const Color4ub& color)
{
	if (mSelColor != color)
	{
		mSelColor = color;
		if (mHasFocus) OnDirty(0, mLayer+1);
	}
}

//============================================================================================================
// Marks this specific widget as needing to be rebuilt
//============================================================================================================

void UIEditableLabel::SetDirty()
{
	const ITexture* tex = GetTexture();
	if (tex != 0) OnDirty(tex, mLayer);
	if (mHasFocus) OnDirty(0, mLayer+1);
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIEditableLabel::OnFill (UIQueue* queue)
{
	byte fontSize ( GetFontSize() );

	if (fontSize > 5)
	{
		if (queue->mLayer	== (mLayer + 1) &&
			queue->mTex		== 0 &&
			queue->mWidget	== 0)
		{
			if (mHasFocus)
			{
				const IFont* font = GetFont();
				Color4ub color ( mSelColor, mRegion.GetCalculatedAlpha() );

				uint selStart, selEnd;
				float left, right;

				// Since mSelectionStart and mSelectionEnd can go either way, figure out which one is less than the other
				if ( mSelectionEnd < mSelectionStart )
				{
					selStart = mSelectionEnd;
					selEnd   = mSelectionStart;
				}
				else
				{
					selStart = mSelectionStart;
					selEnd   = mSelectionEnd;
				}

				// If the selection start is after the line start, figure out the offset in pixels
				if ( selStart > mStart )
				{
					left = mRegion.GetCalculatedLeft() + font->GetLength( mText, mStart, selStart, mTags );
				}
				else
				{
					// The selection start is at or below the line start -- cap it at line start
					selStart = mStart;
					left	 = mRegion.GetCalculatedLeft();
				}

				// Recalculate the end position
				mEnd = mStart + font->CountChars( mText, Float::RoundToUInt(mRegion.GetCalculatedWidth()),
					mStart, 0xFFFFFFFF, false, false, mTags );

				// Limit the selection end is past the right side, cap it
				if ( selEnd > mEnd )  selEnd = mEnd;

				if ( selStart == selEnd )
				{
					// If the selection start and ending points match, make the highlight visible anyway
					right = left + mRegion.GetCalculatedHeight() / 4;
				}
				else
				{
					// Otherwise simply figure out the end of the selection in pixels
					right = left + font->GetLength( mText, selStart, selEnd, mTags );
				}

				float top	 = mRegion.GetCalculatedTop();
				float bottom = mRegion.GetCalculatedBottom();

				Array<IUI::Vertex>& v (queue->mVertices);

				v.Expand().Set( left,  top,		0.0f, 0.0f, color );
				v.Expand().Set( left,  bottom,	0.0f, 1.0f, color );
				v.Expand().Set( right, bottom,	1.0f, 1.0f, color );
				v.Expand().Set( right, top,		1.0f, 0.0f, color );
			}
		}
		else UIBasicLabel::OnFill(queue);
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIEditableLabel::OnSerializeFrom (const TreeNode& node)
{
	if (UITextLine::OnSerializeFrom(node))
	{
		return true;
	}
	else if (node.mTag == "Selection Color")
	{
		Color4ub color;
		if (node.mValue >> color) SetSelectionColor(color);
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIEditableLabel::OnSerializeTo (TreeNode& node) const
{
	UITextLine::OnSerializeTo(node);
	node.AddChild("Selection Color", mSelColor);
}

//============================================================================================================
// Repond to mouse movement
//============================================================================================================

void UIEditableLabel::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if ( mUI->IsKeyDown(Key::MouseLeft) )
	{
		uint index = _GetIndexAt(pos);

		if ( mSelectionEnd != index )
		{
			mSelectionEnd = index;
			OnDirty(0, mLayer+1);
		}
	}
	UIWidget::OnMouseMove(pos, delta);
}

//============================================================================================================
// Respond to key events
//============================================================================================================

void UIEditableLabel::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (isDown)
	{
#ifdef _WINDOWS
		bool ctrl ( mUI->IsKeyDown(Key::LeftControl) ||
					mUI->IsKeyDown(Key::RightControl) );
#else
		bool ctrl ( mUI->IsKeyDown(Key::LeftWindows) ||
					mUI->IsKeyDown(Key::RightWindows) );
#endif

		if		( key == Key::MouseLeft )	_MoveCursorTo( _GetIndexAt(pos) );
		else if ( key == Key::Home )		_MoveCursorTo(0);
		else if ( key == Key::End )			_MoveCursorTo(0xFFFFFFFF);
		else if ( key == Key::ArrowLeft )	_MoveCursorTo( (mSelectionEnd > 0 && !ctrl) ? mSelectionEnd - 1 : 0 );
		else if ( key == Key::ArrowRight )	_MoveCursorTo( ctrl ? 0xFFFFFFFF : mSelectionEnd + 1 );
		else if ( key == Key::Backspace )	OnChar(key);
		else if ( key == Key::Delete )		OnChar(key);
		else if ( key == Key::C && ctrl )
		{
			mUI->SetClipboardText(GetSelectedText());
		}
		else if ( key == Key::X && ctrl )
		{
			mUI->SetClipboardText(GetSelectedText());

			String left, right;
			_GetLeftRight(left, right);

			mText.Clear();
			mText << left;
			mSelectionStart = mText.GetLength();
			mSelectionEnd = mSelectionStart;
			mText << right;
			SetDirty();
		}
		else if ( key == Key::V && ctrl )
		{
			String left, right;
			_GetLeftRight(left, right);

			mText.Clear();
			mText << left;
			mText << mUI->GetClipboardText();
			mSelectionStart = mText.GetLength();
			mSelectionEnd = mSelectionStart;
			mText << right;
			SetDirty();
		}
	}
	else if ( key == Key::Return )
	{
		OnValueChange();

		// Give up focus
		mUI->_SetFocusArea(0);
	}
	else if	( key == Key::Escape )
	{
		mUI->_SetFocusArea(0);
	}
	UIWidget::OnKeyPress(pos, key, isDown);
}

//============================================================================================================
// Select the widget
//============================================================================================================

void UIEditableLabel::OnFocus (bool selected)
{
	mHasFocus = selected;
	OnDirty(0, mLayer+1);
	UIWidget::OnFocus(selected);
}

//============================================================================================================
// Add typed characters to the label's text
//============================================================================================================

void UIEditableLabel::OnChar (byte character)
{
	uint left, right;

	if ( mSelectionEnd < mSelectionStart )
	{
		left  = mSelectionEnd;
		right = mSelectionStart;
	}
	else
	{
		left  = mSelectionStart;
		right = mSelectionEnd;
	}

	if ( character == Key::Delete || character == Key::Backspace )
	{
		if (left == right)
		{
			if (character == Key::Backspace)
			{
				if (left > 0)  --left;
			}
			else if (right < mText.GetLength())  ++right;
		}

		String txtLeft, txtRight;
		mText.GetString(txtLeft, 0, left);
		mText.GetString(txtRight, right);
		mText.Clear();
		mText << txtLeft;
		mText << txtRight;
	}
	else
	{
		String txtLeft, txtRight;
		mText.GetString(txtLeft, 0, left);
		mText.GetString(txtRight, right);
		mText.Clear();
		mText << txtLeft;
		mText.Append("%c", character);
		mText << txtRight;
		++left;
	}

	if (left < mStart || left > mEnd)
	{
		const IFont* font = GetFont();
		ASSERT(font != 0, "Font is missing?");

		// If the point we're deleting up to is left of the rendered text, move it down
		mStart = left - font->CountChars( mText,
			Float::RoundToUInt(mRegion.GetCalculatedWidth()), 0, left, true, false, mTags );
	}

	mSelectionEnd = mSelectionStart = left;

	SetDirty();
	UIWidget::OnChar(character);
}