#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// NOTE: You must add your 3ds Max 9 SDK folders to the configuration lists! Example:
//============================================================================================================
// Tools -> Options -> Projects & Solutions -> VC++ Directories
// Include files: C:\Program Files (x86)\Autodesk\3ds Max 9 SDK\maxsdk\include
// Library files: C:\Program Files (x86)\Autodesk\3ds Max 9 SDK\maxsdk\lib
//============================================================================================================

// R5 Engine include files
#include "../../../Engine/Serialization/Include/_All.h"

// 3ds Max include files
#include <Max.h>
#include <iparamm2.h>
#include <stdmat.h>
#include <CS/bipedapi.h>
#include <CS/phyexp.h>
#include <ISkin.h>

// Missing 3ds max class types
#define DUMMY_CLASSID		::Class_ID(DUMMY_CLASS_ID, 0)
#define TRIOBJ_CLASSID		::Class_ID(TRIOBJ_CLASS_ID, 0)
//#define SKEL_OBJ_CLASSID	::Class_ID(0x9125, 0)
#define BIPED_NODE_CLASSID	::Class_ID(0x9154, 0)
//#define BIPED_CLASSID		::Class_ID(0x9155, 0)
#define BIPED_ROOT_CLASSID	::Class_ID(0x9156, 0)
#define BONE_CLASSID		::Class_ID(BONE_CLASS_ID, 0)

// Pesky global defines that should never be part of the global namespace
#undef min
#undef max
#undef base_type
#undef near
#undef far
#undef IsMinimized
#undef DeleteFont
#undef Mesh

// For simplicity's sake
using namespace R5;

// Application include files
#include "R5 Max Exporter.h"