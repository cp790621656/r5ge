#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Frame buffer object implementation
//============================================================================================================

class GLGraphics;

class GLFBO : public IRenderTarget
{
	struct TextureEntry
	{
		ITexture*	mTex;
		uint		mFormat;

		TextureEntry() : mTex(0), mFormat(ITexture::Format::Invalid) {}
	};

protected:

	friend class GLController;

	IGraphics*			mGraphics;
	mutable uint		mFbo;
	Vector2i			mSize;
	ITexture*			mDepthTex;
	ITexture*			mStencilTex;
	TextureEntry*		mAttachment;
	mutable Array<uint>	mBuffers;
	Color4f				mBackground;
	bool				mUsesSkybox;
	mutable bool		mIsDirty;
	Thread::Lockable	mLock;

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
	
	virtual const Color4f& GetBackgroundColor() const		{ return mBackground; }
	virtual void  SetBackgroundColor (const Color4f& color)	{ mBackground = color; }
	
	virtual bool  IsUsingSkybox() const						{ return mUsesSkybox; }
	virtual void  UseSkybox (bool val)						{ mUsesSkybox = val; }

	virtual bool AttachColorTexture		(uint bufferIndex, ITexture* tex, uint format = ITexture::Format::RGB);
	virtual bool AttachDepthTexture		(ITexture* tex);
	virtual bool AttachStencilTexture	(ITexture* tex);

	virtual bool HasColor()		const;
	virtual bool HasDepth()		const { return mDepthTex != 0; }
	virtual bool HasStencil()	const { return mStencilTex != 0; }

	virtual void Activate()		const;
	virtual void Deactivate()	const;
};