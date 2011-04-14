#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Draw target can be the screen, PBuffer, or a Frame Buffer Object
// Author: Michael Lyashenko
//============================================================================================================

struct IRenderTarget
{
	virtual ~IRenderTarget() {};

	virtual void Release()=0;

	virtual uint GetMaxColorAttachments() const=0;
	virtual bool SupportsStencilAttachments() const=0;

	virtual const Vector2i& GetSize() const=0;
	virtual bool SetSize (const Vector2i& size)=0;

	virtual uint GetMSAA (uint level) const=0;
	virtual void SetMSAA (uint level)=0;

	virtual bool IsUsingSkybox() const=0;
	virtual void UseSkybox (bool val)=0;

	virtual bool AttachColorTexture	(uint bufferIndex, ITexture* tex, uint format = ITexture::Format::RGB)=0;
	virtual bool AttachDepthTexture (ITexture* tex)=0;
	virtual bool AttachStencilTexture (ITexture* tex)=0;

	virtual const ITexture* GetColorTexture (uint bufferIndex) const=0;
	virtual const ITexture* GetDepthTexture () const=0;
	virtual const ITexture* GetStencilTexture () const=0;

	virtual bool HasColor()		const=0;
	virtual bool HasDepth()		const=0;
	virtual bool HasStencil()	const=0;

	virtual void Activate()		const=0;
	virtual void Deactivate()	const=0;

	// Copy the render target's content into the destination buffer
	virtual bool CopyTo (const IRenderTarget* destination, bool color = true, bool depth = true, bool stencil = true) const=0;
};