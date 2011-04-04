#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Shader preprocessing functionality -- only referenced from inside GLSubShader.cpp
//============================================================================================================

namespace R5
{
	// Preprocess all dependencies
	void PreprocessDependencies (String& source, Array<String>& dependencies);

	// Preprocess a new fragment shader format
	uint PreprocessShader (String& source, Flags& flags, bool deferred, bool shadowed);
};