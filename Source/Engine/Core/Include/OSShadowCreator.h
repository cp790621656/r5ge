#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Adds a new rendering step that draws the scene's shadows
//============================================================================================================

class OSShadowCreator : public Script
{
protected:

	Core*			mCore;
	IGraphics*		mGraphics;
	OSSceneRoot*	mRoot;
	Scene			mScene;
	uint			mTextureSize;
	uint			mBlurPasses;
	IRenderTarget*	mLightDepthTarget;
	IRenderTarget*	mShadowTarget;
	ITexture*		mLightDepthTex;
	ITexture*		mShadowTex;

	OSShadowCreator() : mCore(0), mGraphics(0), mRoot(0), mTextureSize(2048), mBlurPasses(1),
		mLightDepthTarget(0), mShadowTarget(0), mLightDepthTex(0), mShadowTex(0) {}

	// Internal functionality
	void DrawLightDepth (const Matrix44& camIMVP);
	void DrawShadows (const ITexture* camDepth);
	void BlurShadows (const ITexture* camDepth, float near, float far);

public:

	R5_DECLARE_INHERITED_CLASS("OSShadowCreator", OSShadowCreator, Script, Script);

	// Add this light to the draw script
	virtual void OnInit();

	// Remove this light from the draw script
	virtual void OnDestroy();

	// Draw the shadow
	ITexture* DrawShadow (const Matrix44& imvp, const ITexture* depth, float near, float far);

	// Serialization
	virtual void OnSerializeTo (TreeNode& node) const;
	virtual void OnSerializeFrom (const TreeNode& node);
};