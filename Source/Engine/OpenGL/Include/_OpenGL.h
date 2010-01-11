#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// OpenGL include header, includes everything needed to make OpenGL calls
//============================================================================================================

#ifdef _WINDOWS
 #include "_Windows_OpenGL.h"
#elif defined (_MACOS)
 #include "_OSX_OpenGL.h"
#endif

#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif

#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8 0x84FA
#endif

#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL 0x84F9
#endif

//==========================================================================================================
// Retrieves some descriptive information about an OpenGL error, if it occured
//==========================================================================================================

const char* glGetErrorString();

//==========================================================================================================
// Queries OpenGL, returning the value. Useful for initializing static variables.
//==========================================================================================================

uint glGetInteger(uint id);

//==========================================================================================================
// Device caps, set on InitOpenGL() and accessible throughout the project
//==========================================================================================================

struct DeviceInfo
{
	struct OS
	{
		enum
		{
			Unknown			= 0x00,
			Windows			= 0x01,
			MacOS			= 0x02,
			Linux			= 0x04,
			WindowsVista	= 0x11,
			Windows7		= 0x21,
		};
	};

	bool	mInitialized;				// Whether the device has been initialized
	byte	mOS;						// Operating system type
	float	mVersion;					// Detected OpenGL version
	bool	mFloat16Format;				// Support for 16-bit floating point textures (half precision)
	bool	mFloat32Format;				// Support for 32-bit floating point textures (full precision)
	bool	mBufferObjects;				// Support for buffer objects (VBOs, FBOs)
	bool	mDrawBuffers;				// Support for multiple color-out attachments for Frame Buffer Objects
	bool	mDepthStencil;				// Support for Depth24-Stencil8 textures and buffers
	bool	mMixedAttachments;			// Support for mixed format FBO color attachments
	bool	mAlphaAttachments;			// Support for alpha textures assigned to FBO as color attachments
	bool	mOcclusion;					// Support for occlusion queries
	bool	mShaders;					// Support for GLSL shaders
	uint	mMaxTextureUnits_FFP;		// Maximum number of texture units that can be used with the fixed-function pipeline
	uint	mMaxTextureUnits_Shader;	// Maximum number of texture units that can be used in shaders
	uint	mMaxTextureCoords;			// Maximum number of texture coordinate arrays
	uint	mMaxTextureSize;			// Maximum texture width and height
	uint	mMaxLights;					// Maximum number of hardware lights
	uint	mMaxFBOAttachments;			// Maximum color attachments for FBOs
	uint	mMaxAnisotropicLevel;		// Maximum supported anisotropic filtering level
	uint	mTextureMemory;				// Amount of videocard memory currently used by textures
	uint	mBufferMemory;				// Amount of videocard memory currently used by VBOs
	uint	mMaxTextureMemory;			// Maximum amount of videocard memory used by textures
	uint	mMaxBufferMemory;			// Maximum amount of videocard memory used by VBOs
	uint	mMaxMemory;					// Maximum amount of combined memory used

	DeviceInfo();
	~DeviceInfo();

	void IncreaseTextureMemory (uint val);
	void IncreaseBufferMemory  (uint val);
	void DecreaseTextureMemory (uint val) { mTextureMemory -= val; }
	void DecreaseBufferMemory  (uint val) { mBufferMemory  -= val; }
};

extern DeviceInfo g_caps;

//==========================================================================================================
// One function to bind all function pointers
//==========================================================================================================

bool InitOpenGL (float requiredVersion);

//==========================================================================================================
// Handy macro to check for an OpenGL error
//==========================================================================================================

#ifdef _DEBUG
	#define CHECK_GL_ERROR						\
	{											\
		const char* err = glGetErrorString();	\
		if (err)								\
		{										\
			WARNING(err);						\
		}										\
	}
#else
	#define CHECK_GL_ERROR
#endif