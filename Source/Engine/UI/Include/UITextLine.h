#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic printable text line with no alignment or boundaries
//============================================================================================================

class UITextLine : public UIWidget
{
protected:

	mutable IFont*	mFont;			// Pointer to the font being used
	String			mText;			// It would be quite odd if the text line was missing actual text
	Color4ub		mTextColor;		// Text can be colored
	Color4ub		mShadowColor;	// Color of the shadow dropped by the text (making it bright == bevel)
	uint			mTags;			// Whether the text processes color tags

public:

	UITextLine() : mTextColor(1.0f), mFont(0), mTags( IFont::Tags::Process ) {}

	const ITexture* GetTexture()	const;
	const String&	GetText()		const	{ return mText;	}
	const Color4ub&	GetTextColor()	const	{ return mTextColor; }
	const Color4ub&	GetShadowColor()const	{ return mShadowColor; }
	const IFont*	GetFont()		const;
	byte			GetFontSize()	const;

	void SetTextColor	(const Color4ub& color);
	void SetShadowColor	(const Color4ub& color);

	virtual void SetText (const String& text);
	virtual void SetFont (const IFont* font);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UITextLine", UITextLine, UIWidget, UIWidget);

	// Mark the associated queue as dirty
	virtual void SetDirty();

	// Set the default font if one hasn't been chosen already
	virtual bool OnUpdate (bool dimensionsChanged);

	// Fill the text draw queue
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};