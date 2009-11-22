#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

#ifndef _GL_GRAPHICS_INCLUDE_H
#define _GL_GRAPHICS_INCLUDE_H

#include "../../Image/Include/_All.h"
#include "../../Font/Include/_All.h"
#include "../../SysWindow/Include/_All.h"

namespace R5
{
	#include "VBO.h"		// OpenGL Vertex Buffer Object
	#include "FBO.h"		// Frame Buffer Object
	#include "Texture.h"	// Regular 2D texture
	#include "Font.h"		// OpenGL implementation of the font class
	#include "Shader.h"		// Vertex and fragment shader management
	#include "Technique.h"	// Material rendering technique
	#include "Material.h"	// Material management
	#include "Controller.h"	// Low-level graphics controller -- closest level of interaction with the renderer API
	#include "Graphics.h"	// Higher level of renderer API interaction, handles resource management for graphical resources
	#include "Window.h"		// OpenGL window creation
};

#endif