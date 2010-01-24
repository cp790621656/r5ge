#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Single vertex or fragment shader, part of a GLSL program
//============================================================================================================

struct GLSubShader
{
	struct Type
	{
		enum
		{
			Invalid		= 0,
			Vertex		= 1,
			Fragment	= 2,
			Geometry	= 3,
		};
	};

	GLGraphics*	mGraphics;		// Graphics manager this shader belongs to
	byte		mType;			// Type of shader this happens to be
	uint		mGLID;			// OpenGL ID of the compiled shader
	String		mSource;		// Source from where the shader came from
	String		mCode;			// Loaded GLSL code the shader was compiled from
	Flags		mFlags;			// Various flags

	// List of dependencies, built on Preprocess()
	Array<GLSubShader*>	mDependencies;

	GLSubShader() : mGraphics(0), mType(Type::Invalid), mGLID(-1) {}
	~GLSubShader();

	// Finds the specified sub-shader
	static String Find (const String& file);

	// Preprocess the shader's source code
	void Preprocess();

	// Validates the shader, compiling it if necessary
	bool IsValid()
	{
		if (mGLID != -1) return true;
		if (mCode.IsEmpty()) return false;
		return _Compile();
	}

	// Adds its own dependencies and dependencies of dependencies to the list
	void AppendDependenciesTo (Array<GLSubShader*>& list);

private:

	// INTERNAL: Compile the shader
	bool _Compile();
};

// This function is used in both GLSubShader as well as GLShader
void PrintDebugLog (const String& log);