#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
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

#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif

//==========================================================================================================
// Library-wide global variable
//==========================================================================================================

struct GLDeviceInfo : public R5::IGraphics::DeviceInfo
{
	// Whether the device has been initialized
	bool mInitialized;

	GLDeviceInfo() : mInitialized(false) {}
	virtual ~GLDeviceInfo();
};

extern GLDeviceInfo g_caps;

//==========================================================================================================
// Retrieves some descriptive information about an OpenGL error, if it occured
//==========================================================================================================

const char* glGetErrorString();

//==========================================================================================================
// Queries OpenGL, returning the value. Useful for initializing static variables.
//==========================================================================================================

uint glGetInteger(uint id);

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