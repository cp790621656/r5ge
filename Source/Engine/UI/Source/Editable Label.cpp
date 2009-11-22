#include "../Include/_All.h"
using namespace R5;

//=============================================================================================
// Get the index of a character at the specified position
//=============================================================================================

uint EditableLabel::_GetIndexAt (const Vector2i& pos) const
{
	if ( mFont != 0 && mText.IsValid() && pos.x > mRegion.GetLeft() )
	{
		uint width = Float::RoundToUInt(pos.x - mRegion.GetLeft());
		uint charIndex = mFont->CountChars( mText, width, mStart, 0xFFFFFFFF, false, true, mTags );
		return mStart + charIndex;
	}
	return mStart;
}

//=============================================================================================
// Moves the cursor to the specified location
//=============================================================================================

void  EditableLabel::_MoveCursorTo(uint index)
{
	if (mFont)
	{
		// 'index' can come in as 0xFFFFFFFF, so be sure to cap it at maximum length
		if (index > mText.GetLength())  index = mText.GetLength();

		bool shiftIsDown = mRoot->IsKeyDown(Key::LeftShift) || mRoot->IsKeyDown(Key::RightShift);
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
			uint count = mFont->CountChars( mText,
				Float::RoundToUInt(mRegion.GetWidth()), 0, index, true, false, mTags );

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

//=============================================================================================
// Retrieves the selected text
//=============================================================================================

String  EditableLabel::GetSelectedText() const
{
	// String class automatically figures out which one is less than the other -- start or end
	String text;
	mText.GetString(text, mSelectionStart, mSelectionEnd);
	return text;
}

//=============================================================================================
// Changes the label's text
//=============================================================================================

void  EditableLabel::SetText (const String& text)
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

//=============================================================================================
// Changes the label's font
//=============================================================================================

void  EditableLabel::SetFont (const IFont* font)
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

//=============================================================================================
// Changes the selection color
//=============================================================================================

void  EditableLabel::SetSelectionColor (const Color4f& color)
{
	if ( mSelColor != color )
	{
		mSelColor = color;
		if (mHasFocus) OnDirty(0, mLayer+1);
	}
}

//=============================================================================================
// Marks this specific area as needing to be rebuilt
//=============================================================================================

void EditableLabel::SetDirty()
{
	const ITexture* tex = GetTexture();
	if (tex != 0) OnDirty(tex, mLayer);
	if (mHasFocus) OnDirty(0, mLayer+1);
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void EditableLabel::OnFill (Queue* queue)
{
	byte fontSize ( GetFontSize() );

	if (fontSize > 5)
	{
		if (queue->mLayer	== (mLayer + 1) &&
			queue->mTex	== 0 &&
			queue->mArea	== 0)
		{
			if (mHasFocus)
			{
				Color4ub color ( mSelColor, mRegion.GetAlpha() );

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
					left = mRegion.GetLeft() + mFont->GetLength( mText, mStart, selStart, mTags );
				}
				else
				{
					// The selection start is at or below the line start -- cap it at line start
					selStart = mStart;
					left	 = mRegion.GetLeft();
				}

				// Recalculate the end position
				mEnd = mStart + mFont->CountChars( mText, Float::RoundToUInt(mRegion.GetWidth()),
					mStart, 0xFFFFFFFF, false, false, mTags );

				// Limit the selection end is past the right side, cap it
				if ( selEnd > mEnd )  selEnd = mEnd;

				if ( selStart == selEnd )
				{
					// If the selection start and ending points match, make the highlight visible anyway
					right = left + mRegion.GetHeight() / 4;
				}
				else
				{
					// Otherwise simply figure out the end of the selection in pixels
					right = left + mFont->GetLength( mText, selStart, selEnd, mTags );
				}

				float top	 = mRegion.GetTop();
				float bottom = mRegion.GetBottom();

				Array<IUI::Vertex>& v (queue->mVertices);

				v.Expand().Set( left,  top,		0.0f, 0.0f, color );
				v.Expand().Set( left,  bottom,	0.0f, 1.0f, color );
				v.Expand().Set( right, bottom,	1.0f, 1.0f, color );
				v.Expand().Set( right, top,		1.0f, 0.0f, color );
			}
		}
		else BasicLabel::OnFill(queue);
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool EditableLabel::CustomSerializeFrom(const TreeNode& root)
{
	if (TextLine::CustomSerializeFrom(root))
	{
		return true;
	}
	else if (root.mTag == "Selection Color")
	{
		Color4f color;
		if (root.mValue >> color) SetSelectionColor(color);
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void EditableLabel::CustomSerializeTo(TreeNode& root) const
{
	TextLine::CustomSerializeTo(root);
	root.AddChild("Selection Color", mSelColor);
}

//============================================================================================================
// Repond to mouse movement
//============================================================================================================

bool EditableLabel::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if ( mRoot->IsKeyDown(Key::MouseLeft) )
	{
		uint index = _GetIndexAt(pos);

		if ( mSelectionEnd != index )
		{
			mSelectionEnd = index;
			OnDirty(0, mLayer+1);
		}
	}
	Area::OnMouseMove(pos, delta);
	return true;
}

//============================================================================================================
// Respond to mouse buttons
//============================================================================================================

bool EditableLabel::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	if (isDown)
	{
		bool commandKey ( mRoot->IsKeyDown(Key::LeftWindows) ||
						  mRoot->IsKeyDown(Key::RightWindows) );

		if		( key == Key::MouseLeft )	_MoveCursorTo( _GetIndexAt(pos) );
		else if ( key == Key::Home )		_MoveCursorTo(0);
		else if ( key == Key::End )			_MoveCursorTo(0xFFFFFFFF);
		else if ( key == Key::ArrowLeft )	_MoveCursorTo( (mSelectionEnd > 0 && !commandKey) ? mSelectionEnd - 1 : 0 );
		else if ( key == Key::ArrowRight )	_MoveCursorTo( commandKey ? 0xFFFFFFFF : mSelectionEnd + 1 );
		else if ( key == Key::Backspace )	OnChar(key);
		else if ( key == Key::Delete )		OnChar(key);
		else if ( key == Key::Return )
		{
			if (mOnValueChange)
			{
				mOnValueChange(this);
			}

			// Give up focus
			mRoot->_SetFocusArea(0);
		}
	}
	else if	( key == Key::Escape )
	{
		mRoot->_SetFocusArea(0);
	}

	Area::OnKey(pos, key, isDown);
	return true;
}

//============================================================================================================
// Select the area
//============================================================================================================

bool EditableLabel::OnFocus (bool selected)
{
	mHasFocus = selected;
	OnDirty(0, mLayer+1);
	Area::OnFocus(selected);
	return true;
}

//============================================================================================================
// Add typed characters to the label's text
//============================================================================================================

bool EditableLabel::OnChar (byte character)
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
		// If the point we're deleting up to is left of the rendered text, move it down
		mStart = left - mFont->CountChars( mText,
			Float::RoundToUInt(mRegion.GetWidth()), 0, left, true, false, mTags );
	}

	mSelectionEnd = mSelectionStart = left;

	SetDirty();
	Area::OnChar(character);
	return true;
}