#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Modifiable left-aligned text label
//============================================================================================================

class EditableLabel : public BasicLabel
{
protected:

	Color4f	mSelColor;			// Selection color
	bool	mHasFocus;			// Whether the area has focus (and thus whether the selection is drawn)
	uint	mSelectionEnd;		// Cursor position inside the text string (an index)
	uint	mSelectionStart;	// Index of the beginning of the selection block (mSelectionEnd being the end)

public:

	EditableLabel() :	mSelColor(1.0f, 0.667f, 0.161f, 0.5f),
						mHasFocus(false),
						mSelectionEnd(0),
						mSelectionStart(0) { mTags = IFont::Tags::Ignore; }

private:

	// Get the index of a character at the specified position
	uint _GetIndexAt (const Vector2i& pos) const;
	
	// Moves the cursor up to the specified index
	void _MoveCursorTo (uint index);

public:

	// These two functions need to reset the selection
	virtual void SetText (const String& text);
	virtual void SetFont (const IFont* font);

	String	  GetSelectedText() const;
	void	  SetSelectionColor (const Color4f& color);
	Color4ub  GetSelectionColor() const { return mSelColor; }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Editable Label", EditableLabel, BasicLabel, Area);

	// Area functions
	virtual void SetDirty();
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;

	// Events
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnFocus	(bool selected);
	virtual bool OnChar		(byte character);
};