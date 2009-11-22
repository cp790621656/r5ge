#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Render target can be the screen, PBuffer, or a Frame Buffer Object, for example
//============================================================================================================

struct IRenderTarget
{
	virtual ~IRenderTarget() {};

	virtual void Release()=0;

	virtual uint GetMaxColorAttachments() const=0;
	virtual bool SupportsStencilAttachments() const=0;

	virtual const Vector2i& GetSize() const=0;
	virtual bool SetSize (const Vector2i& size)=0;

	virtual const Color4f& GetBackgroundColor() const=0;
	virtual void SetBackgroundColor (const Color4f& color)=0;
	
	virtual bool IsUsingSkybox() const=0;
	virtual void UseSkybox (bool val)=0;

	virtual bool AttachColorTexture		(uint bufferIndex, ITexture* tex, uint format = ITexture::Format::RGB)=0;
	virtual bool AttachDepthTexture		(ITexture* tex)=0;
	virtual bool AttachStencilTexture	(ITexture* tex)=0;

	virtual bool HasColor()		const=0;
	virtual bool HasDepth()		const=0;
	virtual bool HasStencil()	const=0;

	virtual void Activate()		const=0;
	virtual void Deactivate()	const=0;
};