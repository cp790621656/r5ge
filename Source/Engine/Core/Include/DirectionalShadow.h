#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Stand-alone class that can be used to draw directional shadows
// Author: Michael Lyashenko
//============================================================================================================

class DirectionalShadow
{
protected:

	IGraphics*		mGraphics;
	uint			mTextureSize;		// Size of each of the shadowmap textures
	uint			mCascadeCount;		// Number of cascades for PSSM
	float			mCascadeBias;		// 0.0 = fully linear cascade distribution, 1.0 = fully quadratic
	uint			mBlurPasses;		// Number of blur passes performed on the final shadow
	float			mSoftness;			// Desired shadow softness
	float			mKernelSize;		// Kernel size when sampling depth around each pixel
	float			mDepthBias;			// Depth sampling bias (allowed depth variance factor)

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
	uint  GetCascadeCount()	 const		{ return mCascadeCount;	}
	float GetCascadeBias()	 const		{ return mCascadeBias;	}
	uint  GetBlurPassCount() const		{ return mBlurPasses;	}
	float GetDepthBias()	 const		{ return mDepthBias;	}
	float GetKernelSize()	 const		{ return mKernelSize;   }

	void SetTextureSize		(uint size)	{ mTextureSize	= size;	}
	void SetCascadeCount	(uint size)	{ mCascadeCount		= size;	}
	void SetCascadeBias		(float val) { mCascadeBias	= val;	}
	void SetBlurPassCount	(uint val)	{ mBlurPasses	= val;	}
	void SetDepthBias		(float val)	{ mDepthBias	= val;	}
	void SetKernelSize		(float val)	{ mKernelSize	= val;	}

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