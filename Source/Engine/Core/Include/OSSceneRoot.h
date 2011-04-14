#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Represents the root of the scene that's used by other scripts
// Author: Michael Lyashenko
//============================================================================================================

class OSSceneRoot : public Script
{
public:

	R5_DECLARE_INHERITED_CLASS("OSSceneRoot", OSSceneRoot, Script, Script);

	// Find the root of the specified parent object, creating a new one if necessary
	static OSSceneRoot* FindRootOf (Object* parent);
};