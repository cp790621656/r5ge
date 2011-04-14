#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Draws the scene using deferred rendering
// Author: Michael Lyashenko
//============================================================================================================

class OSDrawDeferred : public OSDraw
{
protected:

	float		mBloom;
	Vector3f	mFocalRange;
	uint		mAOQuality;
	uint		mAOPasses;
	Vector3f	mAOParams;

	IShader*	mCombine;
	ITexture*	mDepth;
	ITexture*	mNormal;
	ITexture*	mMatDiff;
	ITexture*	mMatParams;
	ITexture*	mLightDiff;
	ITexture*	mLightSpec;
	ITexture*	mFinal;
	Matrix44	mIMVP;

	SSAO		mSSAO;
	PostProcess	mPostProcess;

	IRenderTarget* mMaterialTarget;
	IRenderTarget* mProjTarget;
	IRenderTarget* mLightTarget;
	IRenderTarget* mFinalTarget;

	ITechnique* mProjTechnique;
	bool mProjTargetActive;

	Array<const ITechnique*>	mDeferred;
	Array<const ITechnique*>	mForward;
	Array<bool>					mProcessedLight;

	OSDrawDeferred();

public:

	R5_DECLARE_INHERITED_CLASS("OSDrawDeferred", OSDrawDeferred, OSDraw, Script);

	// Bloom makes areas above the specified threshold appear to glow
	float GetBloom() const { return mBloom; }
	void SetBloom (float val) { mBloom = val; }

	// Focal range for depth-of-field (X = 100% focus distance, Y = range from X that's still 100%, Z = 0%)
	const Vector3f& GetFocalRange() const { return mFocalRange; }
	void SetFocalRange (const Vector3f& range) { mFocalRange = range; }

	// Screen-space ambient occlusion level (0 = off, 1 = low, 2 = high)
	uint GetAOQuality() const { return mAOQuality; }
	void SetAOQuality (uint val) { mAOQuality = val; }

	// Number of blur passes for SSAO
	uint GetAOBlurPasses() const { return mAOPasses; }
	void SetAOBlurPasses (uint val) { mAOPasses = val; }

	// Additional properties for SSAO: X = Range, Y = Strength, Z = Sharpness
	const Vector3f& GetAOParams() const { return mAOParams; }
	void SetAOParams (const Vector3f& v) { mAOParams = v; }

public:

	// Initialize the scene
	virtual void OnInit();
	virtual void OnDestroy() { mScene.Release(); mScene.SetOnDraw(0); }

protected:

	// Material encoding stage
	void MaterialStage();

	// Light encoding stage
	void LightStage();

	// Combine the light contribution with material
	void CombineStage();

	// Add forward-rendered objects and post-process effects
	void PostProcessStage();

public:

	// Draw callback
	virtual void OnDraw();

	// Callback triggered before the technique is activated and objects get drawn
	void OnDrawWithTechnique (const ITechnique* tech);

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual void OnSerializeFrom (const TreeNode& node);
};