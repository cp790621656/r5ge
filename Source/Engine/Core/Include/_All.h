#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#ifndef _CORE_INCLUDE_H
#define _CORE_INCLUDE_H

#include "../../Render/Include/_All.h"

#ifdef _DEBUG
 #ifndef ASSERT_IF_UNLOCKED
  #define ASSERT_IF_UNLOCKED _AssertIfUnlocked()
 #endif
#else
 #define ASSERT_IF_UNLOCKED
#endif

namespace R5
{
	class Object;

	#include "DrawGroup.h"			// Class managing an array of drawable objects
	#include "DrawList.h"			// All drawable objects are separated by techniques into different lists
	#include "DrawLayer.h"			// Drawable objects can be placed on different layers
	#include "DrawQueue.h"			// Draw queue contains all 32 possible draw layers

	#include "Resource.h"			// TreeNode-based resource
	#include "FillParams.h"			// Struct containing parameters passed during the 'fill visible geometry' stage
	#include "Script.h"				// Scripts can be attached to game objects
	#include "Object.h"				// Most basic game object
	#include "Decal.h"				// Projected texture object

	#include "Billboard.h"			// Screen-aligned billboard
	#include "Glare.h"				// Screen-aligned billboard that fades out if it's obscured
	#include "DirectionalBillboard.h"	// Billboard that always appears on the horizon
	#include "DirectionalGlare.h"	// Glare version of the horizon billboard
	#include "DirectionalLight.h"	// Directional light
	#include "PointLight.h"			// Point light
	#include "Emitter.h"			// Particle emitter system template

	#include "Camera.h"				// Base camera class
	#include "DebugCamera.h"		// Free movement camera, useful for debugging
	#include "AnimatedCamera.h"		// Extended camera class supporting spline-based animation
	#include "CameraController.h"	// Camera Controller provides a way to blend together different cameras

	#include "Mesh.h"				// Complete drawable mesh
	#include "Cloud.h"				// Mesh made up of screen-facing billboards (tree canopy, for example)
	#include "Animatable.h"			// Animatable placeable
	#include "Bone.h"				// Single bone for skeletal animation
	#include "Animation.h"			// Keyframed animation
	#include "Skeleton.h"			// Skeleton structure for skeletal animation
	#include "Limb.h"				// Limb class is a mesh+material pair association, used by the Prop class
	#include "BoneTransform.h"		// Skeletal bone transform with relative, absolute, and inverse bind values
	#include "ActiveAnimation.h"	// Currently active animation
	#include "ModelTemplate.h"		// All models are created from templates, which are usually individual files
	#include "Prop.h"				// Placeable static (non-animated) model
	#include "Model.h"				// Model created from the template, contains its own animations
	#include "ModelInstance.h"		// Instantiated model that can be placed into the scenegraph

	#include "QuadNode.h"			// Quad-tree subdivided node, can be extended to create terrains
	#include "QuadTree.h"			// Quad-tree subdivisioned scene object
	#include "TerrainNode.h"		// Subdivisioned child of the Terrain class
	#include "Terrain.h"			// Simple terrain implementation using QuadTree

	#include "BoneAttachment.h"		// Script that binds its owner to the specified bone of the owner's parent

	#include "Scene.h"				// Scene root
	#include "UpdateList.h"			// Container for registered update callbacks
	#include "EventDispatcher.h"	// Event dispatcher has the ability to register event handling callbacks
	#include "Core.h"				// Engine Core
};

#endif