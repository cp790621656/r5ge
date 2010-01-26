#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Single vertex, fragment or fragment shader, part of a GLSL program
//============================================================================================================

class GLSubShader : public ISubShader
{
	friend class GLShader;
	friend class GLGraphics;

protected:

	GLGraphics*	mGraphics;		// Graphics manager this shader belongs to
	byte		mType;			// Type of shader this happens to be
	uint		mGLID;			// OpenGL ID of the compiled shader
	String		mCode;			// Loaded GLSL code the shader was compiled from
	bool		mIsDirty;		// Whether the shader should be recompiled

	// List of dependencies, built on Preprocess()
	Array<GLSubShader*>	mDependencies;

	// Only the GLGraphics class should be creating new shaders
	GLSubShader (GLGraphics* graphics, const String& name, byte type);

private:

	// INTERNAL: Releases the shader
	void _Release();

	// INTERNAL: Preprocess the shader's source code
	void _Preprocess();

	// INTERNAL: Compile the shader
	bool _Compile();

public:

	// Release the associated OpenGL shader when this class gets destroyed
	virtual ~GLSubShader() { _Release(); }

	// Retrieves the source code used to compile this shader
	virtual const String& GetCode() const { return mCode; }

	// Changes the code for the current shader
	virtual void SetCode (const String& code, bool notifyShaders = true);

	// Marks the shader as needing to be recompiled
	virtual void SetDirty() { mIsDirty = true; }

	// Validates the shader, compiling it if necessary
	virtual bool IsValid();

public:

	// Adds its own dependencies and dependencies of dependencies to the list
	void AppendDependenciesTo (Array<GLSubShader*>& list);
};

// This function is used in both GLSubShader as well as GLShader
void PrintDebugLog (const String& log);