#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//===============================================================================================================
// Project-wide device capability information, set on InitOpenGL()
//===============================================================================================================

GLDeviceInfo g_caps;

//===============================================================================================================

GLDeviceInfo::~GLDeviceInfo()
{
#ifdef _DEBUG
	if (mInitialized)
	{
		System::Log("[OPENGL]  Peak memory resourced used:");

		if (mTextureMemory > 0)
		{
			ASSERT(false, "Memory leak detected!");
			System::Log("          - TEXTURE MEMORY LEAK:  %s bytes", String::GetFormattedSize(mTextureMemory).GetBuffer());
		}

		if (mBufferMemory > 0)
		{
			ASSERT(false, "Memory leak detected!");
			System::Log("          - BUFFER MEMORY LEAK:  %s bytes", String::GetFormattedSize(mBufferMemory).GetBuffer());
		}

		System::Log("          - Texture:  %s bytes", String::GetFormattedSize(mMaxTextureMemory).GetBuffer());
		System::Log("          - Buffers:  %s bytes", String::GetFormattedSize(mMaxBufferMemory).GetBuffer());
		System::Log("          - Total:    %s bytes", String::GetFormattedSize(mMaxMemory).GetBuffer());
	}
#endif
}

//===============================================================================================================
// Queries OpenGL, returning the value. Useful for initializing static variables.
//===============================================================================================================

uint glGetInteger(uint id)
{
	int val;
	glGetIntegerv(id, &val);
	return (uint)val;
}

//===============================================================================================================
// Retrieves some descriptive information about an OpenGL error, if it occurred
//===============================================================================================================

const char* glGetErrorString()
{
	int error = glGetError();
	switch (error)
	{
		case 0:										return 0;
		case GL_INVALID_ENUM:						return "An unacceptable value is specified for an enumerated argument";
		case GL_INVALID_VALUE:						return "A numeric argument is out of range";
		case GL_INVALID_OPERATION:					return "The specified operation is not allowed in the current state";
		case GL_STACK_OVERFLOW:						return "This command would cause a stack overflow";
		case GL_STACK_UNDERFLOW:					return "This command would cause a stack underflow";
		case GL_OUT_OF_MEMORY:						return "There is not enough memory left to execute the command";
		case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:	return "Invalid frame buffer operation";
		default:
		{
			System::Log("[OPENGL]  Undocumented error: '0x%x'", error);
			return "Undocumented OpenGL error";
		}
	}
}

//===============================================================================================================
// Checks for the existence of an OpenGL extension
//===============================================================================================================

bool CheckExtension(const char* extension, bool essential = true)
{
	static String list;
	if (list.IsEmpty()) list = (const char*)glGetString(GL_EXTENSIONS);
	if (list.Find(extension) == list.GetLength())
	{
		System::Log("[OPENGL]  %s! Functionality not supported: '%s'", essential ? "ERROR" : "WARNING", extension);
		return false;
	}
	return true;
}

//===============================================================================================================
// Check for depth texture-only attachment support
//===============================================================================================================

bool CheckDepthOnlyAttachmentSupport()
{
	uint tex (0);
	
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERROR;
	
	uint fbo = 0;
	glGenFramebuffers(1, &fbo);
	if (fbo == 0) return false;
	
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex, 0);
	
	bool retVal = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
	
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glDeleteRenderbuffers(1, &fbo);
	glDeleteTextures(1, &tex);
	
	return retVal;
}

//===============================================================================================================
// Check for alpha format frame buffer attachment support
//===============================================================================================================

bool CheckAlphaAttachmentSupport()
{
	uint tex (0);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERROR;

	uint fbo = 0;
	glGenFramebuffers(1, &fbo);
	if (fbo == 0) return false;

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex, 0);

	bool retVal = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glDeleteRenderbuffers(1, &fbo);
	glDeleteTextures(1, &tex);

	return retVal;
}

//===============================================================================================================
// Check for mixed format frame buffer attachment support
//===============================================================================================================

bool CheckMixedAttachmentSupport()
{
	uint tex0 (0);
	uint tex1 (0);

	glGenTextures(1, &tex0);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glGenTextures(1, &tex1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERROR;

	uint fbo = 0;
	glGenFramebuffers(1, &fbo);
	if (fbo == 0) return false;

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, tex1, 0);

	bool retVal = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;

	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glDeleteRenderbuffers(1, &fbo);

	glDeleteTextures(1, &tex1);
	glDeleteTextures(1, &tex0);

	return retVal;
}

//===============================================================================================================
// Calls the function to bind all important OpenGL functions and validates the output
//===============================================================================================================

bool InitOpenGL (float requiredVersion)
{
	if (!g_caps.mInitialized)
	{
		g_caps.mInitialized		= true;
		bool supported			= true;
		bool full				= true;
		const char* renderer	= (const char*)glGetString(GL_RENDERER);
		const char* brand		= (const char*)glGetString(GL_VENDOR);
		const char* versionStr  = (const char*)glGetString(GL_VERSION);
		const char* shaderStr	= (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

		String brandStr (brand);
		if		(brandStr.Contains("NVidia"))	g_caps.mVendor = IGraphics::DeviceInfo::Vendor::NVidia;
		else if (brandStr.Contains("Intel"))	g_caps.mVendor = IGraphics::DeviceInfo::Vendor::Intel;
		else if (brandStr.Contains("ATI"))		g_caps.mVendor = IGraphics::DeviceInfo::Vendor::ATI;

		g_caps.mMaxTextureUnits_FFP		= glGetInteger(GL_MAX_TEXTURE_UNITS);
		g_caps.mMaxTextureUnits_Shader	= glGetInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
		g_caps.mMaxTextureCoords		= glGetInteger(GL_MAX_TEXTURE_COORDS);
		g_caps.mMaxTextureSize			= glGetInteger(GL_MAX_TEXTURE_SIZE);
		g_caps.mMaxLights				= glGetInteger(GL_MAX_LIGHTS);
		g_caps.mMaxFBOAttachments		= glGetInteger(GL_MAX_COLOR_ATTACHMENTS_EXT);
		g_caps.mMaxAnisotropicLevel		= CheckExtension("GL_EXT_texture_filter_anisotropic", false) ?
										  glGetInteger(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT) : 0;

		float reportedVersion (0.0f), shaderVersion (0.0f);
		if (versionStr != 0) sscanf(versionStr, "%f", &reportedVersion);
		if (shaderStr  != 0) sscanf(shaderStr,  "%f", &shaderVersion);

#ifdef _WINDOWS
		supported = _BindFunctionPointers() && supported;
#endif

		if (requiredVersion >= 1.3f)
		{
			supported = CheckExtension("GL_ARB_depth_texture") && supported;

			if (supported)
			{
				g_caps.mVersion = 1.3f;

				if (requiredVersion >= 1.5f)
				{
					g_caps.mFloat16Format	= CheckExtension("GL_ARB_half_float_pixel", false);
					g_caps.mFloat32Format	= CheckExtension("GL_ARB_texture_float", false);
					g_caps.mBufferObjects	= CheckExtension("GL_EXT_framebuffer_object");
					g_caps.mBufferObjects	= CheckExtension("GL_ARB_vertex_buffer_object") && g_caps.mBufferObjects;
					g_caps.mDrawBuffers		= CheckExtension("GL_ARB_draw_buffers");
					g_caps.mDepthStencil	= CheckExtension("GL_EXT_packed_depth_stencil");
					g_caps.mDXTCompression	= CheckExtension("GL_EXT_texture_compression_s3tc", false);
					g_caps.mOcclusion		= CheckExtension("GL_ARB_occlusion", false);

					if (g_caps.mVendor == IGraphics::DeviceInfo::Vendor::ATI)
					{
						// ATI Driver bug: Catalyst does not support Depth24+Stencil8 texture format.
						// Frame buffer objects using such textures exhibit hideous artifacts.
						// Tried this on Catalyst version 9.1, 9.2 and 10.3. Also note:
						// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=254426
						g_caps.mDepthStencil = false;
					}

					if (supported = g_caps.mBufferObjects)
					{
						g_caps.mVersion = 1.5f;
						full &= (g_caps.mFloat16Format && g_caps.mFloat32Format && g_caps.mDrawBuffers && g_caps.mOcclusion);

						if (g_caps.mDrawBuffers)
						{
							g_caps.mDepthAttachments = CheckDepthOnlyAttachmentSupport();
							g_caps.mAlphaAttachments = CheckAlphaAttachmentSupport();
							g_caps.mMixedAttachments = CheckMixedAttachmentSupport();
						}

						if (requiredVersion >= 2.0f)
						{
							g_caps.mShaders = CheckExtension("GL_ARB_shader_objects");
							g_caps.mShaders = CheckExtension("GL_ARB_fragment_shader")	&& g_caps.mShaders;
							g_caps.mShaders = CheckExtension("GL_ARB_vertex_shader")	&& g_caps.mShaders;
							g_caps.mShaders = (shaderVersion >= 1.1f)					&& g_caps.mShaders;

							if (supported = g_caps.mShaders)
							{
								g_caps.mVersion = 2.0f;
								g_caps.mGeometryShaders = CheckExtension("GL_ARB_geometry_shader4", false);

								if (g_caps.mGeometryShaders)
								{
									g_caps.mVersion = 3.2f;
								}
							}
						}
					}
				}
			}
		}

#ifndef _DEBUG
		if (!supported || g_caps.mVersion < 2.0f || !full)
#endif
		{
			System::Log("[OPENGL]  Device driver information:");
			System::Log("          - Videocard:             %s", renderer);
			System::Log("          - Vendor Brand:          %s", brand);
			System::Log("          - Reported Version:      %.1f (%s)", reportedVersion, versionStr);
			System::Log("          - Detected Version:      %.1f (%s)", g_caps.mVersion, full ? "full" : "partial");
			System::Log("          - Shader Version:        %.1f", shaderVersion);
			System::Log("          - FFP Texture Units:     %u", g_caps.mMaxTextureUnits_FFP);
			System::Log("          - Shader Texture Units:  %u", g_caps.mMaxTextureUnits_Shader);
			System::Log("          - Texture Width Limit:   %u", g_caps.mMaxTextureSize);
			System::Log("          - Texture Coordinates:   %u", g_caps.mMaxTextureCoords);
			System::Log("          - Hardware Lights:       %u", g_caps.mMaxLights);
			System::Log("          - FBO Attachments:       %u", g_caps.mMaxFBOAttachments);
			System::Log("          - FBO Depth Attachments: %s", g_caps.mDepthAttachments ? "Supported" : "Not supported");
			System::Log("          - FBO Alpha Attachments: %s", g_caps.mAlphaAttachments ? "Supported" : "Not supported");
			System::Log("          - FBO Mixed Formats:     %s", g_caps.mMixedAttachments ? "Supported" : "Not supported");
			System::Log("          - FBO Packed Stencil:    %s", g_caps.mDepthStencil ? "Supported" : "Not supported");
		}

		if (!supported)
		{
			Thread::MessageWindow("Your videocard driver does not support one or more of the required features.\nCheck the log file for more information.");
		}
	}
	return (g_caps.mVersion >= requiredVersion);
}