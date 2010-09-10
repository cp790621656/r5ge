#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Shader preprocessing functionality -- only referenced from inside GLSubShader.cpp
//============================================================================================================

namespace R5
{
	// Macro that adds skinning support. Example implementations:
	bool PreprocessSkinning (String& source);

	// Macro that adds pseudo-instancing support. Example implementations:
	bool PreprocessInstancing (String& source);

	// Macro that adds billboard cloud transform functionality.
	bool PreprocessBillboarding (String& source);

	// Preprocess deprecated GLSL vertex shader functionality, replacing such things as 'gl_MultiTexCoord1' with
	// their equivalent vertex attribute names.
	void PreprocessAttributes (String& source);

	// Macro that implements common vertex output functionality including adding forward rendering lighting code
	bool PreprocessVertexOutput (String& source, bool deferred);

	// Macro that implements common fragment shader output functionality used by the engine
	bool PreprocessFragmentOutput (String& source, bool deferred, bool shadowed);

	// Preprocess all dependencies
	void PreprocessDependencies (String& source, Array<String>& dependencies);
};