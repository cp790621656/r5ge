#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Represents the root of the scene that's used by other scripts
//============================================================================================================

class OSSceneRoot : public Script
{
public:

	R5_DECLARE_INHERITED_CLASS("OSSceneRoot", OSSceneRoot, Script, Script);

	// Find the root of the specified parent object, creating a new one if necessary
	static OSSceneRoot* FindRootOf (Object* parent);
};