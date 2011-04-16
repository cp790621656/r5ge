#pragma once

//============================================================================================================
//											R5 Game Engine
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// OpenGL Window creation
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _LINUX

class GLWindow : public SysWindow
{
public:

	GLWindow(uint MSAA = 0): SysWindow(MSAA) { }
};

#endif
