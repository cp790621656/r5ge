#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Screen Space Ambient Occlusion post process effect functions
// Author: Michael Lyashenko
//============================================================================================================

class SSAO
{
	ITexture*	mRandom;
	ITechnique*	mPost;

	IShader*	mSSAO;
	IShader*	mBlurH;
	IShader*	mBlurV;

	IRenderTarget* mSSAOTarget;
	IRenderTarget* mBlurTarget0;
	IRenderTarget* mBlurTarget1;

	ITexture* mLightmap;
	ITexture* mBlurTex0;
	ITexture* mBlurTex1;

	void CreateResources (TemporaryStorage& storage);

public:

	SSAO() : mRandom(0), mPost(0), mSSAO(0), mBlurH(0), mBlurV(0),
		mSSAOTarget(0), mBlurTarget0(0), mBlurTarget1(0), mLightmap(0), mBlurTex0(0), mBlurTex1(0) {}

	ITexture* Create (TemporaryStorage& storage, bool highQuality, uint passes,
		float range, float strength, float sharpness);
};

// Generated normalized normap map texture
ITexture* GetRandomizedNormalmap (IGraphics* graphics);