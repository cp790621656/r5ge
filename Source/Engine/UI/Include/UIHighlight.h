#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Most basic visible Root component -- a colored quad
//============================================================================================================

class UIHighlight : public UIWidget
{
protected:

	Color4f	mTopColor;
	Color4f	mBottomColor;

public:

	UIHighlight() : mTopColor(1.0f), mBottomColor(1.0f) {}

	void SetColor (const Color4f& color) { mTopColor = color; mBottomColor = color; OnDirty(0); }

	const Color4f& GetTopColor() const			{ return mTopColor; }
	void SetTopColor (const Color4f& color)		{ mTopColor = color; OnDirty(0); }

	const Color4f& GetBottomColor() const		{ return mBottomColor; }
	void SetBottomColor (const Color4f& color)	{ mBottomColor = color; OnDirty(0); }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Highlight", UIHighlight, UIWidget, UIWidget);

	// Area functions
	virtual void SetDirty()					{ OnDirty(0); }
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;
};