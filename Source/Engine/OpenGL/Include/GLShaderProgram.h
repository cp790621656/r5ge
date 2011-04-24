#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// GLSL shader program using one or more Shader Components
// Author: Michael Lyashenko
//============================================================================================================

struct GLShaderProgram
{
	GLGraphics* mGraphics;

	GLShaderComponent* mVert;
	GLShaderComponent* mFrag;
	GLShaderComponent* mGeom;

	GLShaderUniforms mUniforms;
	Array<Flags> mMatches;

	uint mGLID;
	Flags mFlags;
	bool mIsDirty;

	GLShaderProgram (GLGraphics* graphics) : mGraphics(graphics), mVert(0), mFrag(0), mGeom(0),
		mGLID(0), mIsDirty(true), mUniforms(graphics) {}

	~GLShaderProgram();

	// Activate the shader program
	bool Activate();

	// Deactivate the shader program
	void Deactivate();

	// Detach all shaders from the program
	void Detach();

	// Attach all shaders to the program
	bool Attach();

	// Logs any information found in the linker log
	void LogLinkerStatus (bool retVal);

	// Set the value of the specified uniform
	bool SetUniform (const String& name, const Uniform& uniform) { return mUniforms.Set(mGLID, name, uniform); }

	// Update the specified uniform
	bool UpdateUniform (uint index, const Uniform& uni) const { return mUniforms.Update(index, uni); }

	// Update all the registered uniforms belonging to the specified group
	uint UpdateUniforms (uint group) const { return mUniforms.Update(mGLID, group); }
};