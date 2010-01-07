#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Simple invisible frame that holds rendering queues for all its children
//============================================================================================================

class UIFrame : public UIArea
{
private:

	PointerArray<UIQueue>	mQs;

public:

	// Shows/hides the frame
	void Show (float animTime = 0.15f)	{ SetAlpha(1.0f, animTime); }
	void Hide (float animTime = 0.15f);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Frame", UIFrame, UIArea, UIArea);

	// Marks a rendering queue associated with this texture as being dirty
	virtual void OnDirty (const ITexture* tex, int layer, const UIArea* area);

	// Marks all rendering queues as needing to be rebuilt
	virtual void SetDirty();

	// Updates the rendering queues as necessary, then draws them to the screen
	virtual uint OnDraw();
};