#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Draw the scene using forward rendering
// Author: Michael Lyashenko
//============================================================================================================

class OSDrawForward : public OSDraw
{
protected:

	ITexture*		mShadowmap;
	ITexture*		mDepthTexture;
	IRenderTarget*	mDepthTarget;
	ITechnique*		mOpaque;
	ITechnique*		mShadowed;

	Array<const ITechnique*> mAdditive;

	OSDrawForward() : mShadowmap(0), mDepthTexture(0), mDepthTarget(0), mOpaque(0), mShadowed(0) {}

public:

	R5_DECLARE_INHERITED_CLASS(OSDrawForward, OSDraw, Script);

	// Initialize the scene
	virtual void OnInit();

	// Release the render target
	virtual void OnDestroy();

	// Draw callback
	virtual void OnDraw();
};