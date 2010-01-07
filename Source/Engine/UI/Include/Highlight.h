#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Most basic visible Root component -- a colored quad
//============================================================================================================

class UIHighlight : public UIArea
{
protected:

	Color4f	mColor;

public:

	UIHighlight() : mColor(1.0f) {}

	const Color4f& GetColor() const			{ return mColor; }
	void SetColor (const Color4f& color)	{ mColor = color; OnDirty(0); }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Highlight", UIHighlight, UIArea, UIArea);

	// Area functions
	virtual void SetDirty()					{ OnDirty(0); }
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;
};