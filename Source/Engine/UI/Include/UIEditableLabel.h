#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Modifiable left-aligned text label
//============================================================================================================

class UIEditableLabel : public UIBasicLabel
{
protected:

	Color4ub	mSelColor;			// Selection color
	bool		mHasFocus;			// Whether the widget has focus (and thus whether the selection is drawn)
	uint		mSelectionEnd;		// Cursor position inside the text string (an index)
	uint		mSelectionStart;	// Index of the beginning of the selection block (mSelectionEnd being the end)

public:

	UIEditableLabel();

private:

	// Get the index of a character at the specified position
	uint _GetIndexAt (const Vector2i& pos) const;
	
	// Moves the cursor up to the specified index
	void _MoveCursorTo (uint index);

	// Retrieves the text to the left and to the right of the selection
	void _GetLeftRight (String& txtLeft, String& txtRight);

public:

	// These two functions need to reset the selection
	virtual void SetText (const String& text);
	virtual void SetFont (const IFont* font);

	// Selects the entire text
	void SelectAll();

	// Retrieves the selected part of the text
	String			GetSelectedText() const;
	void			SetSelectionColor (const Color4ub& color);
	const Color4ub& GetSelectionColor() const { return mSelColor; }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIEditableLabel", UIEditableLabel, UIBasicLabel, UIWidget);

	// Area functions
	virtual void SetDirty();
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;

	// Events
	virtual void OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual void OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
	virtual void OnFocus	(bool selected);
	virtual void OnChar		(byte character);
};