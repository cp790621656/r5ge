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
	uint			mCascades;
	uint			mBlurPasses;
	float			mSoftness;
	float			mKernelSize;
	float			mDepthBias;
	IRenderTarget*	mLightDepthTarget[4];
	IRenderTarget*	mShadowTarget;
	IRenderTarget*	mBlurTarget0;
	IRenderTarget*	mBlurTarget1;
	ITexture*		mLightDepthTex[4];
	ITexture*		mDummyColorTex;		// Dummy color texture -- alpha testing doesn't work without it on ATI
	ITexture*		mShadowTex;
	ITexture*		mBlurTex0;
	IShader*		mShader[4];
	IShader*		mBlurH;
	IShader*		mBlurV;
	ITechnique*		mPost;

	// Internal functionality
	void DrawLightDepth (Object* root, const Object* eye, const Vector3f& dir, const Matrix44& camIMVP);
	void DrawShadows (const ITexture* camDepth);
	void BlurShadows (const ITexture* camDepth);

public:

	DirectionalShadow();

	void Initialize (IGraphics* graphics);
	void Release();

	// Access to protected variables
	uint  GetTextureSize()	 const		{ return mTextureSize;	}
	uint  GetBlurPassCount() const		{ return mBlurPasses;	}
	float GetDepthBias()	 const		{ return mDepthBias;	}
	float GetKernelSize()	 const		{ return mKernelSize;   }
	void  SetBlurPassCount(uint val)	{ mBlurPasses	= val;	}
	void  SetTextureSize  (uint size)	{ mTextureSize	= size;	}
	void  SetDepthBias	  (float val)	{ mDepthBias	= val;	}
	void  SetKernelSize	  (float val)	{ mKernelSize	= val;	}

	// Draw the shadow
	ITexture* Draw (
		Object*			root,		// Root of the scene
		const Object*	eye,		// Object representing the eye of the shadow (used as an identifier)
		const Vector3f& dir,		// Direction of the light
		const Matrix44& imvp,		// Camera's inverse modelview-projection matrix
		const ITexture* depth);		// Camera's depth

	// Serialization
	void OnSerializeTo (TreeNode& node) const;
	void OnSerializeFrom (const TreeNode& node);
};