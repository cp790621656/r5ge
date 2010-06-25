#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Draw the scene using forward rendering
//============================================================================================================

class OSDrawForward : public OSDraw
{
protected:

	Camera*				mCam;
	ITexture*			mDepthTexture;
	IRenderTarget*		mDepthTarget;
	DirectionalShadow	mShadow;

	OSDrawForward() : mCam(0), mDepthTexture(0), mDepthTarget(0) {}

public:

	R5_DECLARE_INHERITED_CLASS("OSDrawForward", OSDrawForward, OSDraw, Script);

	// Initialize the scene
	virtual void OnInit();

	// Release the render target
	virtual void OnDestroy();

	// Draw callback
	virtual void OnDraw();
};