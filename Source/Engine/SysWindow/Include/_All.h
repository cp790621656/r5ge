#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

#ifndef _SYSTEM_WINDOW_INCLUDE_H
#define _SYSTEM_WINDOW_INCLUDE_H

#include "../../Interface/Include/_All.h"

#ifdef _WINDOWS
	#include "_MSWindow.h"
#elif defined (_MACOS)
	#include "_CocoaWindow.h"
#else
	#error "Unsupported system configuration"
#endif // _WINDOWS

#endif // _SYSTEM_WINDOW_INCLUDE_H