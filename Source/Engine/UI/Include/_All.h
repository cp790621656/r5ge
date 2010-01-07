#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Graphic User Interface
//============================================================================================================

#ifndef _R5_UI2_INCLUDE_ALL_H
#define _R5_UI2_INCLUDE_ALL_H

#include "../../Interface/Include/_All.h"

namespace R5
{
	#include "Queue.h"				// Rendering queue
	#include "Anchor.h"				// Simple struct that combines relative and absolute coordinates
	#include "Region.h"				// Basic anchored 2D region
	#include "Face.h"				// Struct containing a pair of UV texture coordinates
	#include "Skin.h"				// Skin is a collection of faces associated with a texture
	#include "Event Handler.h"		// Event handler class containing functors for event callbacks
	
	#include "Area.h"				// Hierarchical GUI node, based on EventHandler, holding a region
	#include "Frame.h"				// Simple invisible frame that holds rendering queues for all its children
	#include "Highlight.h"			// Most basic visible UI component -- a colored quad
	#include "Picture.h"			// Simple textured quad
	#include "SubPicture.h"			// Drawable Face
	#include "Slider.h"				// Slider widget
	#include "Text Line.h"			// Basic printable text line with no alignment or boundaries
	#include "Text Area.h"			// Multi-paragraph text area
	#include "Basic Label.h"		// Expanded text line, adding start and end boundaries
	#include "Label.h"				// Expanded BasicLabel, adding left/center/right alignment options
	#include "Button.h"				// Basic button (SubPicture + Label)
	#include "Checkbox.h"			// Checkbox (Slightly modified sticky button)
	#include "Editable Label.h"		// Modifiable left-aligned text label
	#include "Input.h"				// Input field (SubPicture + Editable Label)
	#include "ShadedArea.h"			// Area affected by a shader

	#include "Animated Frame.h"		// Frame that smoothly fades in and out when alpha changes
	#include "Animated Slider.h"	// Slider that smoothly animated to its set value
	#include "Animated Button.h"	// Button that smoothly fades from one state to the next
	#include "Animated Checkbox.h"	// Same as the animated button, just based on the Checkbox class

	#include "Context.h"			// Context menu
	#include "Menu.h"				// Drop-down menu, animated button + context menu
	#include "List.h"				// Slightly extended menu class that updates the button's text

	#include "Window.h"				// Window is a complex widget based on Animated Frame, using several other widgets
	#include "Root.h"				// User interface root

	#include "Templates.h"			// FindWidget<> and AddWidget<> function templates
	#include "UI.h"					// R5 engine-based implementation of the Root
};

#endif