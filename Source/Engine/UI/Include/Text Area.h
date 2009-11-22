#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Text Area containing paragraphs of text
//============================================================================================================

class UITextArea : public UIArea
{
protected:

	// Text drawing style
	struct Style
	{
		enum
		{
			Normal	= 0,
			Chat	= 1,
		};
	};

	// Single paragraph entry
	struct Paragraph
	{
		Color3f			mColor;		// Text can be colored
		const IFont*	mFont;		// Pointer to the font being used
		String			mText;		// It would be quite odd if the text line was missing actual text
		bool			mShadow;	// Whether the text has a shadow outline

		Paragraph() : mColor(1.0f), mFont(0), mShadow(true) {}
		void Release() { mText.Release(); }
	};

protected:

	Array<Paragraph>		mParagraphs;	// Paragraph entries
	PointerArray<UITextLine>	mLines;			// Generated lines based on the paragraphs above

	bool	mNeedsRebuild;	// Whether the paragraph lines need to be rebuilt
	float	mScroll;		// Current scrolling amount determining visible text
	uint	mStyle;			// Whether chat style paragraphs are used (bottom to top)
	uint	mHeight;		// Combined height of lines, in pixels
	uint	mMaxParagraphs;	// Maximum allowed number of paragraphs before old ones get removed

public:

	UITextArea() :
		mScroll			(0.0f),
		mNeedsRebuild	(false),
		mStyle			(Style::Normal),
		mHeight			(0),
		mMaxParagraphs	(255) {}

	uint	GetMaxParagraphs()	const { return mMaxParagraphs;	}
	uint	GetStyle()			const { return mStyle;			}
	float	GetScroll()			const { return mScroll;			}

	// Clears all text
	void Clear();

	// Maximum number of allowed paragraphs
	void SetMaxParagraphs (uint val) { mMaxParagraphs = val; }

	// Adds a new paragraph of specified font and color
	void AddParagraph (const String& text, const IFont* font, const Color3f& color, bool shadow = true);

	// Paragraph drawing style (normal being top-down, chatbox being bottom up, etc)
	void SetStyle (uint style) { mStyle = style; _MarkTexturesAsDirty(); }

	// Scroll amount
	void SetScroll (float val);

private:

	// INTERNAL: Marks all associated textures as dirty
	void _MarkTexturesAsDirty();

	// INTERNAL: Rebuilds the printed lines
	void _Rebuild (uint offset);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Text Area", UITextArea, UIArea, UIArea);

	// Area functions
	virtual void SetDirty();
	virtual void OnFill (UIQueue* queue);
	virtual bool OnScroll (const Vector2i& pos, float delta);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};