#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// OpenGL surface shader implementation
// Author: Michael Lyashenko
//============================================================================================================

class GLSurfaceShader : public IShader
{
	struct RegisteredUniform
	{
		String				mName;		// Name of the uniform variable (ie: "R5_eyePos")
		SetUniformDelegate	mDelegate;	// Delegate function that will set the data for that uniform
		uint				mGroup;		// Optional group for uniforms that's referenced in Update()

		RegisteredUniform() : mGroup(Uniform::Group::SetWhenActivated) {}
	};

	GLGraphics* mGraphics;
	CodeNode	mCode;
	Flags		mBasicFlags;
	Flags		mActiveFlags;
	bool		mCheckForSource;
	bool		mIsDirty;
	bool		mHasErrors;

	// Different shaders (vertex, fragment, geometry), with different variations
	PointerArray<GLShaderComponent> mComponents;

	// Created shader programs
	PointerArray<GLShaderProgram> mPrograms;

	// Registered uniforms
	Array<RegisteredUniform> mUniforms;

private:

	friend class GLGraphics;
	friend class GLController;

	GLSurfaceShader (GLGraphics* graphics, const String& name) :
		mGraphics(graphics), mCheckForSource(true), mIsDirty(false), mHasErrors(true) { mName = name; }

	// Activate this shader for the specified technique
	bool Activate (const ITechnique* tech);

	// Deactivate the currently active shader
	void Deactivate() const;

	// Try to find the source code for this shader
	void CheckForSource();

	// Set the shader code directly
	void SerializeFrom (const String& code);

	// Get a compiled variation of the shader
	Flags GetVariation (String& out, const Flags& flags) const;

	// Gets or creates a shader program given the specified set of flags
	GLShaderProgram* GetProgram (const Flags& flags);

	// Gets or creates a shader component given the specified set of flags
	GLShaderComponent* GetComponent (const Flags& flags);

	// Update the specified uniform group
	void UpdateUniforms (uint group) const;

public:

	// Release all shaders, then all programs
	virtual ~GLSurfaceShader();

	// Should match against the active shader's flags
	virtual bool GetFlag (uint val) const;

	// Clears the shader, making it invalid
	virtual void Clear() { mCode.Release(); mIsDirty = true; }

	// Marks the shader as needing to be rebuilt
	virtual void SetDirty() { mIsDirty = true; }

	// Sets the shader's source code
	virtual void SetCode (const String& code);

	// Retrieves the shader's source code
	virtual void GetCode (String& out) const { out.Clear(); return mCode.SerializeTo(out); }

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const;

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint group);
};
