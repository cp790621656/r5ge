#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Most basic visible Root component -- a colored quad
//============================================================================================================

class Highlight : public Area
{
protected:

	Color4f	mColor;

public:

	Highlight() : mColor(1.0f) {}

	const Color4f& GetColor() const			{ return mColor; }
	void SetColor (const Color4f& color)	{ mColor = color; OnDirty(0); }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Highlight", Highlight, Area, Area);

	// Area functions
	virtual void SetDirty()					{ OnDirty(0); }
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
	virtual void CustomSerializeTo (TreeNode& root) const;
};