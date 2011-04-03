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