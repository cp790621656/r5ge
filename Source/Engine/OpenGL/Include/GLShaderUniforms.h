#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Built-in shader uniforms
// Author: Michael Lyashenko
//============================================================================================================

struct GLShaderUniforms
{
	typedef IShader::SetUniformDelegate SetUniformDelegate;

	struct Entry
	{
		String				mName;		// Name of the uniform variable (ie: "R5_eyePos")
		SetUniformDelegate	mDelegate;	// Delegate function that will set the data for that uniform
		mutable int			mGLID;		// OpenGL resource ID in the shader, once found (-2 if not checked, -1 if not found)
		uint				mGroup;		// Optional group for uniforms that's referenced in Update()

		Entry() : mGLID(-2), mGroup(Uniform::Group::SetWhenActivated) {}
	};

	GLGraphics* mGraphics;
	Array<Entry> mList;

private:

	// Delegate functions for common shader uniforms
	void SetUniform_EyePos			(const String& name, Uniform& uniform);
	void SetUniform_PixelSize		(const String& name, Uniform& uniform);
	void SetUniform_ClipRange		(const String& name, Uniform& uniform);
	void SetUniform_FogRange		(const String& name, Uniform& uniform);
	void SetUniform_FogColor		(const String& name, Uniform& uniform);

	void SetUniform_LightAmbient	(const String& name, Uniform& uniform);
	void SetUniform_LightDiffuse	(const String& name, Uniform& uniform);
	void SetUniform_LightPosition	(const String& name, Uniform& uniform);
	void SetUniform_LightDirection	(const String& name, Uniform& uniform);
	void SetUniform_LightParams		(const String& name, Uniform& uniform);

	void SetUniform_MatColor		(const String& name, Uniform& uniform);
	void SetUniform_MatParams0		(const String& name, Uniform& uniform);
	void SetUniform_MatParams1		(const String& name, Uniform& uniform);

	void SetUniform_MS				(const String& name, Uniform& uniform);
	void SetUniform_MM				(const String& name, Uniform& uniform);
	void SetUniform_VM				(const String& name, Uniform& uniform);
	void SetUniform_PM				(const String& name, Uniform& uniform);
	void SetUniform_MVM				(const String& name, Uniform& uniform);
	void SetUniform_MVPM			(const String& name, Uniform& uniform);
	void SetUniform_IVM				(const String& name, Uniform& uniform);
	void SetUniform_IVRM			(const String& name, Uniform& uniform);
	void SetUniform_IPM				(const String& name, Uniform& uniform);
	void SetUniform_IMVPM			(const String& name, Uniform& uniform);

public:

	GLShaderUniforms (GLGraphics* graphics) : mGraphics(graphics) { RegisterBuiltInUniforms(); }

	// Add a new registered uniform value without checking to see if it already exists
	void Insert (const String& name, const SetUniformDelegate& fnct, uint group);

	// Register all built-in uniforms
	void RegisterBuiltInUniforms();

	// Bind all one-time attributes prior to linking the program
	void PreLink (uint program);

	// Set the constant values of texture units in the shader
	void PostLink (uint program);

	// Set the value of the specified uniform
	bool Set (uint program, const String& name, const Uniform& uniform);

	// Update the specified group of uniforms
	uint Update (uint program, uint group) const;

	// Update the specified group of uniforms
	bool Update (uint index, const Uniform& uni) const;

	// Registers a uniform variable that's updated once per frame
	void Register (const String& name, const SetUniformDelegate& fnct, uint group);
};