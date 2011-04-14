#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Rendering queue
// Author: Michael Lyashenko
//============================================================================================================

class UIWidget;
struct UIQueue
{
	const ITexture*		mTex;			// Queue's associated texture
	const UIWidget*		mWidget;			// If specified, this queue will be for this widget only
	Array<IUI::Vertex>	mVertices;		// Array of vertices that will be placed into the VBO
	bool				mDynamic;		// Whether the queue is expected to change frequently
	bool				mIsDirty;		// Whether the queue should be rebuilt next frame
	int					mLayer;			// Layer used for sorting purposes lowest is drawn first
	bool				mIgnoreAlpha;	// Whether alpha channel is ignored

	UIQueue () :	mTex		 (0),
					mWidget		 (0),
					mDynamic	 (false),
					mIsDirty	 (false),
					mLayer		 (0),
					mIgnoreAlpha (false) {}

	virtual ~UIQueue() {}

	virtual bool IsValid() const { return mVertices.IsValid(); }
};