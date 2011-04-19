#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Various post-processing effects
// Author: Michael Lyashenko
//============================================================================================================

class PostProcess
{
	IGraphics*	mGraphics;
	ITechnique* mPostProcess;
	IShader*	mBlurH;
	IShader*	mBlurV;
	IShader*	mDOF;
	IShader*	mCombine;
	IShader*	mBloomBlur;

	IRenderTarget*	mTarget00;
	IRenderTarget*	mTarget01;
	IRenderTarget*	mTarget10;
	IRenderTarget*	mTarget11;
	IRenderTarget*	mTarget20;
	IRenderTarget*	mTarget21;

	ITexture*		mTexture00;
	ITexture*		mTexture01;
	ITexture*		mTexture10;
	ITexture*		mTexture11;
	ITexture*		mTexture20;
	ITexture*		mTexture21;

public:

	PostProcess();

	// Initialize local resources
	void Initialize (IGraphics* graphics);

protected:

	// Blurring effect -- Half size
	void BlurDownsample (TemporaryStorage&	storage,
						 const ITexture*	depth,
						 const IShader*		replacement,
						 const IShader*		postProcess);

public:

	// No post-processing
	void None (TemporaryStorage& storage);

	// Bloom post-processing effect
	void Bloom (TemporaryStorage& storage, float threshold);

	// Depth of field effect
	// X = distance to center, Y = 100% focus distance (from center), Z = 0% focus distance (from center)
	void DepthOfField (TemporaryStorage& storage, const Vector3f& focalRange);

	// Both depth of field as well as bloom effects
	void Both (TemporaryStorage& storage, float threshold, const Vector3f& focalRange);
};