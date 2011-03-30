#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Graphic User Interface
//============================================================================================================

#ifndef _R5_UI2_INCLUDE_ALL_H
#define _R5_UI2_INCLUDE_ALL_H

#include "../../Interface/Include/_All.h"

namespace R5
{
	#include "UIQueue.h"			// Rendering queue
	#include "UIAnchor.h"			// Simple struct that combines relative and absolute coordinates
	#include "UIRegion.h"			// Basic anchored 2D region
	#include "UIFace.h"				// Struct containing a pair of UV texture coordinates
	#include "UISkin.h"				// Skin is a collection of faces associated with a texture
	
	#include "UIScript.h"			// Script that can be attached to the widget
	#include "USEventListener.h"	// Event listener is a UI Script that can forward events to delegates
	#include "UIWidget.h"			// Hierarchical GUI node
	#include "UIFrame.h"			// Simple invisible frame that holds rendering queues for all its children
	#include "UIHighlight.h"		// Most basic visible UI component -- a colored quad
	#include "UIPicture.h"			// Simple textured quad
	#include "UISubPicture.h"		// Drawable Face
	#include "UISlider.h"			// Slider widget
	#include "UITextLine.h"			// Basic printable text line with no alignment or boundaries
	#include "UITextArea.h"			// Multi-paragraph text widget
	#include "UIBasicLabel.h"		// Expanded text line, adding start and end boundaries
	#include "UILabel.h"			// Expanded UIBasicLabel, adding left/center/right alignment options
	#include "UIButton.h"			// Basic button (SubPicture + Label)
	#include "UICheckbox.h"			// Checkbox (Slightly modified sticky button)
	#include "UIEditableLabel.h"	// Modifiable left-aligned text label
	#include "UIInput.h"			// Input field (SubPicture + Editable Label)
	#include "UIShadedArea.h"		// Area affected by a shader
	#include "UIColorPicker.h"		// UIPicture containing a color gradient and 2 sliders (luminance, alpha)

	#include "UIAnimatedFrame.h"	// Frame that smoothly fades in and out when alpha changes
	#include "UIAnimatedSlider.h"	// Slider that smoothly animated to its set value
	#include "UIAnimatedButton.h"	// Button that smoothly fades from one state to the next
	#include "UIAnimatedCheckbox.h"	// Same as the animated button, just based on the Checkbox class

	#include "UIContext.h"			// Context menu
	#include "UIMenu.h"				// Drop-down menu, animated button + context menu
	#include "UIList.h"				// Slightly extended menu class that updates the button's text
	#include "UITreeView.h"			// Widget designed to visualize TreeNode hierarchies
	#include "UIStats.h"			// Debugger widget -- contains various run-time statistics

	#include "USFadeIn.h"			// Script that fades in the widget
	#include "USFadeOut.h"			// Script that fades out the widget

	#include "UIWindow.h"			// Window is a complex widget based on Animated Frame, using several other widgets
	#include "UIManager.h"			// User interface manager
	#include "UI.h"					// R5 engine-based implementation of the UIManager
};

#endif