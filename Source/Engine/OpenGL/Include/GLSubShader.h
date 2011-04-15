#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Single vertex, fragment or fragment shader, part of a GLSL program
// Author: Michael Lyashenko
//============================================================================================================

class GLSubShader
{
	friend class GLShader;
	friend class GLGraphics;

public:

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

protected:

	GLGraphics*	mGraphics;		// Graphics manager this shader belongs to
	GLShader*	mShader;		// Shader program this subshader belongs to
	String		mName;			// This shader's name
	Flags		mFlags;			// Flags associated with this shader
	byte		mType;			// Type of shader this happens to be
	uint		mGLID;			// OpenGL ID of the compiled shader
	String		mCode;			// Loaded GLSL code the shader was compiled from
	bool		mIsDirty;		// Whether the shader should be recompiled

	// Only the GLGraphics class should be creating new shaders
	GLSubShader (GLGraphics* graphics, GLShader* shader, const String& name);

private:

	// INTERNAL: Releases the shader
	void _Release();

	// INTERNAL: Compile the shader
	bool _Compile();

public:

	// Release the associated OpenGL shader when this class gets destroyed
	~GLSubShader() { _Release(); }

	// Retrieves the source code used to compile this shader
	const String& GetCode() const { return mCode; }

	// Changes the code for the current shader
	void SetCode (const String& code, const Flags& flags = Flags());

	// Marks the shader as needing to be recompiled
	void SetDirty() { mIsDirty = true; }

	// Validates the shader, compiling it if necessary
	bool IsValid();
};

// This function is used in both GLSubShader as well as GLShader
void CreateDebugLog (Array<String>& lines, const String& log, const String& code);