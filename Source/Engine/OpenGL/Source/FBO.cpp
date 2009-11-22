#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Keep track of the active buffer
//============================================================================================================

uint g_activeBuffer = 0;

//============================================================================================================
// Function that checks whether the FBO is marked as ready
//============================================================================================================

bool IsFBOComplete (bool displayError = true)
{
	// Check the frame buffer's status
	uint result = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);

	if (result != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		if (displayError)
		{
			const char* message = "Unknown";

			switch (result)
			{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
				message = "Not all framebuffer attachments are in a complete state";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				message = "No attachments were associated with the frame buffer";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				message = "Not all attachments have identical width and height";
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
				message = "The requested combination of attachments is not supported on your hardware";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				message = "Missing a call to 'glDrawBuffers'";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				message = "Format mismatch";
				break;
			}
			ASSERT(false, message);
		}
		return false;
	}
	return true;
}

//============================================================================================================
// Delayed callback executed by the GLGraphics manager (buffers should only be released on the graphics thread)
//============================================================================================================

void DeleteFBO(void* ptr)
{
	uint fbo = (uint)(ulong)ptr;

	if (g_activeBuffer == fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, g_activeBuffer = 0);
	}
	glDeleteFramebuffers(1, &fbo);
}

//============================================================================================================
// FBO constructor
//============================================================================================================

FBO::FBO(IGraphics* graphics) :
	mGraphics	(graphics),
	mFbo		(0),
	mDepthTex	(0),
	mStencilTex	(0),
	mAttachment	(0),
	mUsesSkybox	(true),
	mIsDirty	(true)
{
	mBackground.Set(0, 0, 0, 1);
	ASSERT( g_caps.mMaxFBOAttachments != 0, "Frame Buffer Objects are not supported?" );
	mAttachment = new TextureEntry[ g_caps.mMaxFBOAttachments ];
}

//============================================================================================================
// Releases the resources associated with the FBO
//============================================================================================================

void FBO::_InternalRelease(bool delayExecution)
{
	if (mAttachment != 0)
	{
		delete [] mAttachment;
		mAttachment = 0;
	}

	if (mFbo != 0)
	{
		if (delayExecution)
		{
			mGraphics->AddDelayedDelegate(DeleteFBO, (void*)mFbo);
		}
		else
		{
			// Optimized version for when we are inside the graphics thread
			DeleteFBO((void*)mFbo);
		}
		mFbo = 0;
	}
}

//============================================================================================================
// Returns the maximum number of color attachments
//============================================================================================================

uint FBO::GetMaxColorAttachments() const
{
	return g_caps.mMaxFBOAttachments;
}

//============================================================================================================
// Whether stencil buffer attachments are supported -- only via the GL_EXT_packed_depth_stencil
//============================================================================================================

bool FBO::SupportsStencilAttachments() const
{
	return g_caps.mDepthStencil;
}

//============================================================================================================
// Changes the buffer's size
//============================================================================================================

bool FBO::SetSize( const Vector2i& size )
{
	if ( mSize != size )
	{
		if ( g_caps.mMaxFBOAttachments == 0 )
		{
			ASSERT(false, "Your videocard does not support Frame Buffer Objects");
			return false;
		}

		if ( size.x < 8 || size.y < 8 )
		{
			ASSERT(false, "The requested size for FBO::SetSize() is too small");
			return false;
		}
		
		if ( size.x > (short)g_caps.mMaxTextureSize ||
			 size.y > (short)g_caps.mMaxTextureSize )
		{
			ASSERT(false, "The requested size for FBO::SetSize() exceeds your hardware's capabilities");
			return false;
		}

		mLock.Lock();
		{
			mSize = size;
			mIsDirty = true;
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Attaches a texture to the specified color index
//============================================================================================================

bool FBO::AttachColorTexture( uint bufferIndex, ITexture* tex, uint format )
{
	if ( bufferIndex < g_caps.mMaxFBOAttachments )
	{
		if ( tex != mAttachment[bufferIndex].mTex )
		{
			mLock.Lock();
			{
				TextureEntry& entry (mAttachment[bufferIndex]);
				entry.mTex		= tex;
				entry.mFormat	= format;
				mIsDirty		= true;

				// If this is a valid format (and it really should be...)
				if (format != ITexture::Format::Invalid)
				{
					bool revert = false;

					// If revert attachments are not supported
					if (!g_caps.mMixedAttachments)
					{
						// Check to see if we have formats that don't match
						for (uint i = 0; i < g_caps.mMaxFBOAttachments; ++i)
						{
							TextureEntry& current (mAttachment[i]);

							if (current.mFormat != format &&
								current.mFormat != ITexture::Format::Invalid)
							{
								revert = true;
								break;
							}
						}
					}

					// If the alpha texture attachments are not supported, we want to revert to RGBA
					if (!revert && !g_caps.mAlphaAttachments && format == ITexture::Format::Alpha)
					{
						revert = true;
					}

					// Revert to RGBA if either mixed format or alpha attachments are not supported
					if (revert)
					{
						for (uint i = 0; i < g_caps.mMaxFBOAttachments; ++i)
						{
							TextureEntry& current (mAttachment[i]);
							if (current.mFormat != ITexture::Format::Invalid)
								current.mFormat  = ITexture::Format::RGBA;
						}
					}
				}
			}
			mLock.Unlock();
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Attaches a depth texture
//============================================================================================================

bool FBO::AttachDepthTexture (ITexture* tex)
{
	if (mDepthTex != tex)
	{
		mLock.Lock();
		{
			mIsDirty = true;
			mDepthTex = tex;
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Attaches a stencil texture
//============================================================================================================

bool FBO::AttachStencilTexture (ITexture* tex)
{
	if (mStencilTex != tex)
	{
		// FBOs only support combined depth+stencil textures. Stand-alone stencil buffers are not supported.
		if ( (!g_caps.mDepthStencil && tex != 0) || (mDepthTex != 0 && mDepthTex != tex) )
			return false;

		// Depth must be attached in order for this to work
		if (mDepthTex == 0) AttachDepthTexture(tex);

		mLock.Lock();
		{
			mIsDirty = true;
			mStencilTex = tex;
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Determines if we have a valid color attachment
//============================================================================================================

bool FBO::HasColor() const
{
	for (uint i = 0; i < g_caps.mMaxFBOAttachments; ++i)
	{
		const ITexture* tex ( mAttachment[i].mTex );
		if (tex != 0) return true;
	}
	return false;
}

//============================================================================================================
// Activates (or deactivates) the buffer
//============================================================================================================

void FBO::Activate() const
{
	// If the buffer isn't valid, don't bother
	if ( mSize == 0 ) return;
	if ( mFbo  == 0 )
	{
		glGenFramebuffers(1, &mFbo);
		ASSERT( mFbo != 0, "Failed to create a FBO!" );
	}

	// If this isn't the currently active buffer, bind it
	if (g_activeBuffer != mFbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, g_activeBuffer = mFbo);
		CHECK_GL_ERROR;
	}

	// NOTE: Supposedly some ATI drivers have issues if FBO textures are not rebound every frame...
	if ( mIsDirty )
	{
		mLock.Lock();
		{
			mIsDirty = false;
			Array<uint>	buffers;
			
			// Clear mask is passed to glClear()
			uint clearMask = 0;

			// Run through all attachments and bind their textures
			for (uint i = 0; i < g_caps.mMaxFBOAttachments; ++i)
			{
				ITexture* tex ( mAttachment[i].mTex );

				if (tex != 0)
				{
					clearMask |= GL_COLOR_BUFFER_BIT;
					uint format = mAttachment[i].mFormat;

					// Auto-adjust unsupported RGB30A2 textures to be RGBA
					if (!g_caps.mMixedAttachments)
					{
						if (ITexture::Format::RGB30A2 == format &&
							ITexture::Format::RGBA == tex->GetFormat())
						{
							format = ITexture::Format::RGBA;
						}
					}

					ASSERT(tex->GetFormat() == ITexture::Format::Invalid || tex->GetFormat() == format,
						"Changing color attachment's format type... is this intentional?");

					if ( mSize != tex->GetSize() )
					{
						uint filter = tex->GetFiltering();

						if (filter > ITexture::Filter::Nearest)
						{
							// If the filter is set to a value above "Nearest", use linear filtering
							filter = ITexture::Filter::Linear;
						}
						else
						{
							// Otherwise assume the default value -- no filtering
							filter = ITexture::Filter::Nearest;
						}

						tex->Set(0, mSize.x, mSize.y, 1, format, format );
						tex->SetFiltering(filter);
						tex->SetWrapMode(ITexture::WrapMode::ClampToEdge);
					}

					buffers.Expand() = GL_COLOR_ATTACHMENT0_EXT + i;
					glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,
						GL_TEXTURE_2D, tex->GetTextureID(), 0);
					CHECK_GL_ERROR;
				}
				else
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, 0, 0);
				}
			}

			// Determine the appropriate attachment formats
			uint depthFormat   = ITexture::Format::Invalid;
			uint stencilFormat = ITexture::Format::Invalid;

			if (mDepthTex != 0)
			{
				if (g_caps.mDepthStencil && mDepthTex == mStencilTex)
				{
					depthFormat	  = ITexture::Format::DepthStencil;
					stencilFormat = ITexture::Format::DepthStencil;
				}
				else
				{
					depthFormat = ITexture::Format::Depth;
				}
			}

			// Attach the depth buffer
			if (depthFormat == ITexture::Format::Invalid)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
			}
			else
			{
				ASSERT(	mDepthTex->GetFormat() == ITexture::Format::Invalid ||
						mDepthTex->GetFormat() == depthFormat,
						"Changing depth attachment's format type... is this intentional?");

				clearMask |= GL_DEPTH_BUFFER_BIT;

				if (mDepthTex->GetSize() != mSize || mDepthTex->GetFormat() != depthFormat)
				{
					mDepthTex->Set(0, mSize.x, mSize.y, 1, depthFormat, depthFormat);
					mDepthTex->SetFiltering(ITexture::Filter::Nearest);
					mDepthTex->SetWrapMode(ITexture::WrapMode::ClampToOne);
				}

				glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D,
					mDepthTex->GetTextureID(), 0);

				CHECK_GL_ERROR;
			}

			// Attach the stencil buffer
			if (stencilFormat == ITexture::Format::Invalid)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
			}
			else
			{
				ASSERT(	mStencilTex->GetFormat() == ITexture::Format::Invalid ||
						mStencilTex->GetFormat() == stencilFormat,
						"Changing stencil attachment's format type... is this intentional?");

				clearMask |= GL_STENCIL_BUFFER_BIT;

				if (mStencilTex->GetSize() != mSize || mStencilTex->GetFormat() != stencilFormat)
				{
					mStencilTex->Set(0, mSize.x, mSize.y, 1, stencilFormat, stencilFormat);
					mStencilTex->SetFiltering(ITexture::Filter::Nearest);
					mStencilTex->SetWrapMode(ITexture::WrapMode::ClampToOne);
				}

				glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D,
					mStencilTex->GetTextureID(), 0);

				CHECK_GL_ERROR;
			}

			// If there were attachments to work with, activate them all
			if (buffers.IsValid())
			{
				// Specify which buffers will be drawn to
				glDrawBuffers(buffers.GetSize(), buffers);
				CHECK_GL_ERROR;
			}
			else
			{
				// Color information should be discarded
				glDrawBuffer(GL_NONE);
				CHECK_GL_ERROR;
			}

			if (IsFBOComplete())
			{
				// Since textures were reset, everything should be cleared -- just in case.
				// Not doing so causes some very odd artifacts -- took me a day to track this down.
				if (clearMask != 0)
				{
					if ((clearMask & GL_COLOR_BUFFER_BIT) != 0)
					{
						glClearColor( mBackground.r, mBackground.g, mBackground.b, mBackground.a );
					}
					glClear(clearMask);
					CHECK_GL_ERROR;
				}
#ifdef _DEBUG
				System::Log("[FBO]     Attachments were relinked for FBO #%u", mFbo);
				System::Log("          - Color:   %u/%u", buffers.GetSize(), g_caps.mMaxFBOAttachments);
				System::Log("          - Depth:   %u/1", (depthFormat != ITexture::Format::Invalid) ? 1 : 0);
				System::Log("          - Stencil: %u/1", (stencilFormat != ITexture::Format::Invalid) ? 1 : 0);
#endif
			}
		}
		mLock.Unlock();
	}
}

//============================================================================================================
// Deactivates the current frame buffer
//============================================================================================================

void FBO::Deactivate() const
{
	if (g_activeBuffer != 0)
	{
		//glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, 0, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, g_activeBuffer = 0);
	}
}