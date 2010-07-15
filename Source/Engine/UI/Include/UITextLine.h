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

	mutable IFont*	mFont;		// Pointer to the font being used
	String			mText;		// It would be quite odd if the text line was missing actual text
	Color4ub		mColor;		// Text can be colored
	bool			mShadow;	// Whether the text has a shadow outline
	uint			mTags;		// Whether the text processes color tags

public:

	UITextLine() : mColor(1.0f), mFont(0), mShadow(false), mTags( IFont::Tags::Process ) {}

	const ITexture* GetTexture()	const;
	const Color4ub&	GetTextColor()	const	{ return mColor;  }
	const String&	GetText()		const	{ return mText;	  }
	const IFont*	GetFont()		const;
	byte			GetFontSize()	const;
	bool			GetShadow()		const	{ return mShadow; }
	Color4ub		GetShadowColor()const	{ return Color4ub(0, 0, 0, Float::ToRangeByte(mRegion.GetCalculatedAlpha()) ); }

	void SetTextColor	(const Color4ub& color);
	void SetShadow		(bool val);
	
	virtual void SetText (const String& text);
	virtual void SetFont (const IFont* font);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UITextLine", UITextLine, UIWidget, UIWidget);

	// Area functions
	virtual void SetDirty();
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};