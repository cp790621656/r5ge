#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// GLSL shader program using one or more SubShaders
//============================================================================================================

class GLShader : public IShader
{
private:

	struct UniformEntry
	{
		String				mName;		// Name of the uniform variable (ie: "R5_eyePos")
		SetUniformDelegate	mDelegate;	// Delegate function that will set the data for that uniform
		int					mGLID;		// OpenGL resource ID in the shader, once found (-2 if not checked, -1 if not found)

		UniformEntry() : mGLID(-2) {}
	};

private:

	GLGraphics*		mGraphics;
	uint			mProgram;

	// Array of shaders used to compile this program
	Array<GLSubShader*> mSubShaders;

	// Registered values
	mutable Array<UniformEntry> mUniforms;

private:

	// Allow the graphics classes to call these functions
	friend class GLGraphics;
	friend class GLController;

	// Should only be accessible through GLGraphics
	bool Init (GLGraphics* graphics, const String& name);

	// Activates this shader
	bool Activate (bool resetUniforms);

	// Deactivates the current shader
	void Deactivate() const;

	// INTERNAL: Link all shaders
	bool _Link();

	// INTERNAL: Updates registered uniforms bound to the shader
	void _UpdateUniforms();

public:

	GLShader() : mGraphics(0), mProgram(0) {}
	virtual ~GLShader();

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const { return mSubShaders.IsValid(); }

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const;

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct);
};