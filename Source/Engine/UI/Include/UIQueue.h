#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Rendering queue
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