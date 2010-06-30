#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Stand-alone class that can be used to draw directional shadows
//============================================================================================================

class DirectionalShadow
{
protected:

	Core*			mCore;
	IGraphics*		mGraphics;
	uint			mTextureSize;
	uint			mBlurPasses;
	IRenderTarget*	mLightDepthTarget;
	IRenderTarget*	mShadowTarget;
	IRenderTarget*	mBlurTarget0;
	IRenderTarget*	mBlurTarget1;
	ITexture*		mLightDepthTex;
	ITexture*		mShadowTex;
	ITexture*		mBlurTex0;
	IShader*		mShadow;
	IShader*		mBlurH;
	IShader*		mBlurV;
	ITechnique*		mPost;

	// Internal functionality
	void DrawLightDepth (Object* root, const Vector3f& dir, const Matrix44& camIMVP);
	void DrawShadows (const ITexture* camDepth);
	void BlurShadows (const ITexture* camDepth);

public:

	DirectionalShadow();

	void Initialize (IGraphics* graphics);
	void Release();

	// Access to protected variables
	uint GetTextureSize() const			{ return mTextureSize; }
	void SetTextureSize (uint size)		{ mTextureSize = size; }
	uint GetBlurPassCount() const		{ return mBlurPasses; }
	void SetBlurPassCount (uint val)	{ mBlurPasses = val; }

	// Draw the shadow
	ITexture* Draw (
		Object*			root,	// Root of the scene
		const Vector3f& dir,	// Direction of the light
		const Matrix44& imvp,	// Camera's inverse modelview-projection matrix
		const ITexture* depth);	// Camera's depth

	// Serialization
	void OnSerializeTo (TreeNode& node) const;
	void OnSerializeFrom (const TreeNode& node);
};