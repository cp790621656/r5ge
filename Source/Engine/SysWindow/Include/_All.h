#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================

#ifndef _SYSTEM_WINDOW_INCLUDE_H
#define _SYSTEM_WINDOW_INCLUDE_H

#include "../../Interface/Include/_All.h"

#ifdef _WINDOWS
	#include "_MSWindow.h"
#elif defined (_MACOS)
	#include "_CocoaWindow.h"
#elif defined (_LINUX)
	#include "_X11Window.h"
#else
	#error "Unsupported system configuration"
#endif // _WINDOWS

#endif // _SYSTEM_WINDOW_INCLUDE_H
