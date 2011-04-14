#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Shader preprocessing functionality -- only referenced from inside GLSubShader.cpp
// Author: Michael Lyashenko
//============================================================================================================

namespace R5
{
	// Preprocess all dependencies
	void PreprocessDependencies (String& source, Array<String>& dependencies);

	// Preprocess a new fragment shader format
	uint PreprocessShader (String& source, Flags& flags, bool deferred, bool shadowed);
};