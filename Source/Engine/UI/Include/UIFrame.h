#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple invisible frame that holds rendering queues for all its children
//============================================================================================================

class UIFrame : public UIWidget
{
private:

	PointerArray<UIQueue>	mQs;

public:

	// Shows/hides the frame
	void Show (float animTime = 0.15f)	{ SetAlpha(1.0f, animTime); }
	void Hide (float animTime = 0.15f);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIFrame", UIFrame, UIWidget, UIWidget);

	// Marks a rendering queue associated with this texture as being dirty
	virtual void OnDirty (const ITexture* tex, int layer, const UIWidget* widget);

	// Marks all rendering queues as needing to be rebuilt
	virtual void SetDirty();

	// Updates the rendering queues as necessary, then draws them to the screen
	virtual uint OnDraw();
};