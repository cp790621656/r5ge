#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Represents the root of the scene that's used by other scripts
// Author: Michael Lyashenko
//============================================================================================================

class OSSceneRoot : public Script
{
public:

	R5_DECLARE_INHERITED_CLASS(OSSceneRoot, Script, Script);

	// Find the root of the specified parent object, creating a new one if necessary
	static OSSceneRoot* FindRootOf (Object* parent);
};