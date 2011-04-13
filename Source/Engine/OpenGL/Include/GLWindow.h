#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c)2008 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

#ifdef _WINDOWS
 #include "_Windows_Window.h"
#elif defined (_MACOS)
 #include "_OSX_Window.h"
#elif defined (_LINUX)
 #include "_X11_Window.h"
#endif
