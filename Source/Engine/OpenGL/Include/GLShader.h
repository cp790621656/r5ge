#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// GLSL shader program using one or more SubShaders
// Author: Michael Lyashenko
//============================================================================================================

class GLShader : public IShader
{
public:

	struct UniformRecord
	{
		String name;
		uint elements;
	};

private:

	struct UniformEntry
	{
		String				mName;		// Name of the uniform variable (ie: "R5_eyePos")
		SetUniformDelegate	mDelegate;	// Delegate function that will set the data for that uniform
		int					mGLID;		// OpenGL resource ID in the shader, once found (-2 if not checked, -1 if not found)
		uint				mGroup;		// Optional group for uniforms that's referenced in Update()

		UniformEntry() : mGLID(-2), mGroup(Uniform::Group::SetWhenActivated) {}
	};

private:

	GLGraphics*	mGraphics;
	uint		mProgram;
	bool		mIsDirty;

	PointerArray<GLShader> mSpecial;	// Array of specially compiled shaders
	PointerArray<GLSubShader> mAdded;	// Array of shaders that will be attached on the next _Activate()
	Array<GLSubShader*> mAttached;		// Array of shaders currently attached to the program

	// Registered values
	mutable Array<UniformEntry> mUniforms;

	// Each shader can actually result in several different versions compiled for specific tasks.
	// Deferred rendering is a variation of a surface shader for example. Forward rendered shader
	// with shadows is another. Each variation is meant to handle a specific task.

	struct Variation
	{
		Flags flags;
		GLShader* shader;
		Variation() : shader(0) {}
	};

	// Alternate versions of this shader (for surface shaders)
	mutable Array<Variation> mVariations;

private:

	// Allow the graphics classes to call these functions
	friend class GLGraphics;
	friend class GLController;

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

private:

	// Should only be used in GLShader
	void _Init (GLGraphics* graphics);

	// Should only be used in GLGraphics
	bool _Init (GLGraphics* graphics, const String& name);

	// Activates this shader for the specified technique
	GLShader* _Activate (const ITechnique* tech);

	// Updates all uniforms
	uint _Update (uint group) const;

	// Deactivates the current shader
	void _Deactivate() const;

private:

	// INTERNAL: Activates this shader
	bool _Activate();

	// INTERNAL: Rebuild the shader program
	bool _Rebuild();

	// INTERNAL: Appends the specified shader to the list
	void _Append (const String& filename);

	// INTERNAL: Detaches the attached shaders
	void _Detach();

	// INTERNAL: Releases all resources used by the shader
	void _Release();

	// INTERNAL: Validate the specified list of shaders
	bool _Validate (PointerArray<GLSubShader>& list);

	// INTERNAL: Attach all GLSL shaders to this shader program
	void _Attach (PointerArray<GLSubShader>& list);

	// INTERNAL: Link all shaders
	bool _Link();

	// INTERNAL: Updates registered uniforms bound to the shader
	void _UpdateUniforms();

	// INTERNAL: Updates a single uniform entry
	bool _UpdateUniform (uint glID, const Uniform& uni) const;

	// INTERNAL: Adds a new registered uniform value without checking to see if it already exists
	void _InsertUniform (const String& name, uint elements, const SetUniformDelegate& fnct, uint group);

	// Adds the specified sub-shader to this program
	void _AddSubShader (GLSubShader* sub);

public:

	GLShader() : mGraphics(0), mProgram(0), mIsDirty(false) {}
	virtual ~GLShader() {}

	// Clears the shader, making it invalid
	virtual void Clear() { mAdded.Clear(); mIsDirty = true; }

	// Marks the shader as needing to be relinked
	virtual void SetDirty() { mIsDirty = true; }

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const;

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const;

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint group);
};
