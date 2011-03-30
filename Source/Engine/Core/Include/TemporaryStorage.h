#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Managed temporary render targets and textures that can be used by draw functions
//============================================================================================================

class TemporaryStorage
{
public:

	typedef Array<IRenderTarget*>	Targets;
	typedef Array<ITexture*>		Textures;
	typedef Array<IVBO*>			VBOs;

protected:

	IGraphics*	mGraphics;		// Cached
	Textures	mTempTextures;	// Cached textures
	Targets		mTempTargets;	// Cached render targets
	VBOs		mTempVBOs;		// Cached vertex buffer objects

	ITexture*	mDepth;			// Final depth texture
	ITexture*	mNormal;		// Final normal texture
	ITexture*	mColor;			// Final color texture
	ITexture*	mShadow;		// Final shadow texture
	ITexture*	mAO;			// Final ambient occlusion texture

	IRenderTarget* mTarget;

public:

	TemporaryStorage() : mGraphics(0), mDepth(0), mNormal(0), mColor(0), mShadow(0), mAO(0), mTarget(0) {}

	void Initialize (IGraphics* graphics);
	void Release();

	IGraphics*		GetGraphics()	{ return mGraphics; }
	ITexture*		GetRenderTexture(uint index);
	IRenderTarget*	GetRenderTarget (uint index);
	IVBO*			GetVBO			(uint index);
	Vector2i		GetFinalTargetSize()	{ return mTarget == 0 ? mGraphics->GetViewport() : mTarget->GetSize(); }

	void ClearFinalTextures()
	{
		mDepth	= 0;
		mColor	= 0;
		mNormal = 0;
		mShadow = 0;
		mAO		= 0;
	}

	void SetDepth	(ITexture* tex) { mDepth	= tex; }
	void SetNormal	(ITexture* tex) { mNormal	= tex; }
	void SetColor	(ITexture* tex) { mColor	= tex; }
	void SetShadow	(ITexture* tex) { mShadow	= tex; }
	void SetAO		(ITexture* tex) { mAO		= tex; }

	ITexture* GetDepth()	{ return mDepth; }
	ITexture* GetNormal()	{ return mNormal; }
	ITexture* GetColor()	{ return mColor; }
	ITexture* GetShadow()	{ return mShadow; }
	ITexture* GetAO()		{ return mAO; }

	const ITexture* GetDepth()	const { return mDepth; }
	const ITexture* GetColor()	const { return mColor; }
	const ITexture* GetNormal()	const { return mNormal; }
	const ITexture* GetShadow()	const { return mShadow; }
	const ITexture* GetAO()		const { return mAO; }

	// Render target associated with the scene
	IRenderTarget* GetFinalTarget() { return mTarget; }
	const IRenderTarget* GetFinalTarget() const { return mTarget; }
	void SetFinalTarget (IRenderTarget* target) { mTarget = target; }
};