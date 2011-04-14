#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Most basic visible Root component -- a colored quad
// Author: Michael Lyashenko
//============================================================================================================

class UIHighlight : public UIWidget
{
protected:

	Color4ub mTopColor;
	Color4ub mBottomColor;

public:

	UIHighlight() : mTopColor(0xFFFFFFFF), mBottomColor(0xFFFFFFFF) {}

	void SetColor (const Color4ub& color)		{ mTopColor = color; mBottomColor = color; OnDirty(0); }

	const Color4ub& GetTopColor() const			{ return mTopColor; }
	void SetTopColor (const Color4ub& color)	{ if (color != mTopColor) { mTopColor = color; OnDirty(0); } }

	const Color4ub& GetBottomColor() const		{ return mBottomColor; }
	void SetBottomColor (const Color4ub& color)	{ if (color != mBottomColor) { mBottomColor = color; OnDirty(0); } }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIHighlight", UIHighlight, UIWidget, UIWidget);

	// Area functions
	virtual void SetDirty() { OnDirty(0); }
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};