#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#ifndef _INTERFACE_INCLUDE_H
#define _INTERFACE_INCLUDE_H

#include "../../Serialization/Include/_All.h"
#include "Defines.h"

namespace R5
{
	#include "IEventReceiver.h"			// Struct with OnMouseMove, OnKeyPress, etc -- basic event receiver interface
	#include "IWindow.h"				// Base class for the application window
	#include "FrameStats.h"				// Frame statistics struct -- used by both UI as well as Graphics
	#include "IUI.h"					// User Interface

	#include "ISoundInstance.h"			// Base class for the sound instance class
	#include "ISound.h"					// Base class for the sound class
	#include "IAudio.h"					// Base class for the audio controller class

	#include "ITechnique.h"				// Rendering technique is used to batch objects of similar properties together
	#include "ITexture.h"				// Base class for textures (Images stored on the videocard)
	#include "IVBO.h"					// Interface for the videocard memory buffer
	#include "IFont.h"					// Basic interface for the Font class
	#include "Uniform.h"				// Uniform is a constant value in the shader that is set by the program
	#include "ILight.h"					// Basic light source-related data class
	#include "ISubShader.h"				// Single shader -- vertex, fragment, or geometry
	#include "IShader.h"				// Complete shader program
	#include "IMaterial.h"				// Base class for the material
	#include "IRenderTarget.h"			// Draw target can be the screen, PBuffer, or a Frame Buffer Object, for example

	#include "IGraphicsController.h"	// Low level graphics card controller interface
	#include "IGraphicsManager.h"		// Higher level graphics resource manager interface
	#include "IGraphics.h"				// Combines IGraphicsController and IGraphicsManager into a single interface
};

#endif