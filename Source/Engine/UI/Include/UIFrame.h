#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Simple invisible frame that holds rendering queues for all its children
// Author: Michael Lyashenko
//============================================================================================================

class UIFrame : public UIWidget
{
public:

	typedef Rectangle<int> Rect;

private:

	PointerArray<UIQueue>	mQs;

public:

	// Shows/hides the frame
	void Show (float animTime = 0.15f)	{ SetAlpha(1.0f, animTime); }
	void Hide (float animTime = 0.15f);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIFrame", UIFrame, UIWidget, UIWidget);

	// Clipping rectangle set before drawing the contents of the frame
	virtual Rect GetClipRect() const;

	// Marks a rendering queue associated with this texture as being dirty
	virtual void OnDirty (const ITexture* tex, int layer, const UIWidget* widget);

	// Marks all rendering queues as needing to be rebuilt
	virtual void SetDirty();

	// Updates the rendering queues as necessary, then draws them to the screen
	virtual uint OnDraw();
};