#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

#ifndef _CORE_INCLUDE_H
#define _CORE_INCLUDE_H

#include "../../Interface/Include/_All.h"

namespace R5
{
	#include "Mesh.h"				// Complete drawable mesh
	#include "Script.h"				// Basic script

	class Core;

	#include "Object.h"				// Scenegraph object implementation
	#include "Animatable.h"			// Animatable placeable
	#include "QuadTree.h"			// Quad-tree split scene object, can be extended to create terrains
	#include "Bone.h"				// Single bone for skeletal animation
	#include "Animation.h"			// Keyframed animation
	#include "Skeleton.h"			// Skeleton structure for skeletal animation
	
	#include "Camera.h"				// Base camera class
	#include "DebugCamera.h"		// Free movement camera, useful for debugging
	#include "AnimatedCamera.h"		// Extended camera class supporting spline-based animation
	#include "CameraController.h"	// Camera Controller provides a way to blend together different cameras
	
	#include "Glow.h"				// Screen-aligned billboard
	#include "DirectionalLight.h"	// Directional light
	#include "PointLight.h"			// Point light
	#include "Emitter.h"			// Particle emitter system template

	#include "Limb.h"				// Limb class is a mesh+material pair association, used by the Prop class
	#include "BoneTransform.h"		// Skeletal bone transform with relative, absolute, and inverse bind values
	#include "ActiveAnimation.h"	// Currently active animation
	#include "ModelTemplate.h"		// All models are created from templates, which are usually individual files
	#include "Prop.h"				// Placeable static (non-animated) model
	#include "Model.h"				// Model created from the template, contains its own animations
	#include "ModelInstance.h"		// Instantiated model that can be placed into the scenegraph

	#include "Scene.h"				// Scene root
	#include "Templates.h"			// Templated FindWidget<> and AddWidget<> functions
	#include "UpdateList.h"			// Container for registered update callbacks
	#include "EventDispatcher.h"	// Event dispatcher has the ability to register event handling callbacks
	#include "Core.h"				// Engine Core
};

#endif