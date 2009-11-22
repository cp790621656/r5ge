#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic printable text line with no alignment or boundaries
//============================================================================================================

class TextLine : public Area
{
protected:

	Color3f	mColor;		// Text can be colored
	IFont*	mFont;		// Pointer to the font being used
	String	mText;		// It would be quite odd if the text line was missing actual text
	bool	mShadow;	// Whether the text has a shadow outline
	uint	mTags;		// Whether the text processes color tags

public:

	TextLine() : mColor(1.0f), mFont(0), mShadow(true), mTags( IFont::Tags::Process ) {}

	const ITexture* GetTexture()	const	{ return (mFont != 0) ? mFont->GetTexture() : 0; }
	const Color3f&	GetColor()		const	{ return mColor;	}
	const String&	GetText()		const	{ return mText;	}
	const IFont*	GetFont()		const	{ return mFont;	}
	byte			GetFontSize()	const	{ return (mFont == 0) ? 0 : mFont->GetSize(); }
	bool			DropsShadow()	const	{ return mShadow;  }
	Color4ub		GetShadowColor()const	{ return Color4ub(0, 0, 0, Float::ToRangeByte(mRegion.GetAlpha()) ); }

	void SetColor	(uint color)			{ SetColor(Color3f(color)); }
	void SetColor	(const Color4ub& color) { SetColor(Color3f(color.mVal)); }
	void SetColor	(const Color3f& color);
	void SetShadow	(bool val);
	
	virtual void SetText	(const String& text);
	virtual void SetFont	(const IFont* font);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Text Line", TextLine, Area, Area);

	// Area functions
	virtual void SetDirty();
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};