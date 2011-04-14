#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Frame buffer object implementation
// Author: Michael Lyashenko
//============================================================================================================

class GLGraphics;

class GLFBO : public IRenderTarget
{
	struct TextureEntry
	{
		ITexture*			mTex;
		mutable ITexture*	mActive;
		uint				mFormat;
		bool				mAntiAlias;

		TextureEntry() : mTex(0), mActive(0), mFormat(ITexture::Format::Invalid) {}
	};
protected:

	friend class GLController;

	IGraphics*			mGraphics;
	mutable uint		mFbo;
	Vector2i			mSize;
	ITexture*			mDepthTex;
	ITexture*			mStencilTex;
	ITexture*			mDummyTex;
	Array<TextureEntry> mAttachments;
	mutable Array<uint>	mBuffers;
	uint				mMSAA;
	bool				mUsesSkybox;
	mutable bool		mIsDirty;
	Thread::Lockable	mLock;

	// Keep track of active textures
	mutable Array<ITexture*> mActiveTextures;

private:

	// Only the R5::GLGraphics class should be able to create FBOs
	friend class R5::GLGraphics;
	GLFBO(IGraphics* graphics);

	void _InternalRelease(bool delayExecution);

public:

	virtual ~GLFBO() { _InternalRelease(false); }

public:

	virtual void Release() { _InternalRelease(true); }

	virtual uint GetMaxColorAttachments() const;
	virtual bool SupportsStencilAttachments() const;

	virtual const Vector2i& GetSize() const					{ return mSize; }
	virtual bool  SetSize (const Vector2i& size);

	virtual uint GetMSAA (uint level) const					{ return mMSAA; }
	virtual void SetMSAA (uint level)						{ mMSAA = level; }
	
	virtual bool  IsUsingSkybox() const						{ return mUsesSkybox; }
	virtual void  UseSkybox (bool val)						{ mUsesSkybox = val; }

	virtual bool AttachColorTexture (uint bufferIndex, ITexture* tex, uint format = ITexture::Format::RGB);
	virtual bool AttachDepthTexture	(ITexture* tex);
	virtual bool AttachStencilTexture (ITexture* tex);

	virtual const ITexture* GetColorTexture (uint bufferIndex) const { return (bufferIndex < mAttachments.GetSize()) ? mAttachments[bufferIndex].mTex : 0; }
	virtual const ITexture* GetDepthTexture () const { return mDepthTex; }
	virtual const ITexture* GetStencilTexture () const { return mStencilTex; }

	virtual bool HasColor()		const;
	virtual bool HasDepth()		const { return mDepthTex != 0; }
	virtual bool HasStencil()	const { return mStencilTex != 0; }

	virtual void Activate()		const;
	virtual void Deactivate()	const;

	// Copy the render target's content into the destination buffer
	virtual bool CopyTo (const IRenderTarget* destination, bool color = true, bool depth = true, bool stencil = true) const;
};