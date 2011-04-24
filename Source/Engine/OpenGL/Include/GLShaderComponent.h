#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Single vertex, fragment or fragment shader, part of a GLSL program
// Author: Michael Lyashenko
//============================================================================================================

struct GLShaderComponent
{
	GLGraphics*	mGraphics;		// Graphics manager this shader belongs to
	String		mName;			// This shader's name
	Flags		mFlags;			// Flags associated with this shader
	uint		mGLID;			// OpenGL ID of the compiled shader
	String		mCode;			// Loaded GLSL code the shader was compiled from
	bool		mIsDirty;		// Whether the shader should be recompiled

private:

	// INTERNAL: Releases the shader
	void _Release();

	// INTERNAL: Compile the shader
	bool _Compile();

public:

	// Create a new shader component
	GLShaderComponent (const String& name);

	// Release the associated OpenGL shader when this class gets destroyed
	~GLShaderComponent() { _Release(); }

	// Retrieves the source code used to compile this shader
	const String& GetCode() const { return mCode; }

	// Changes the code for the current shader
	void SetCode (const String& code, const Flags& flags);

	// Marks the shader as needing to be recompiled
	void SetDirty() { mIsDirty = true; }

	// Validates the shader, compiling it if necessary
	bool IsValid();
};

// This function is used in both GLSurfaceShader as well as GLShaderComponent
void CreateDebugLog (Array<String>& lines, const String& log, const String& code);
