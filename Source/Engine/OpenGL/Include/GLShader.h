#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// GLSL shader program using one or more SubShaders
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

		UniformEntry() : mGLID(-2) {}
	};

private:

	GLGraphics*	mGraphics;
	uint		mProgram;
	bool		mIsDirty;

	Array<GLSubShader*> mAdded;		// Array of shaders that will be attached on the next Activate()
	Array<GLSubShader*> mDepended;	// Array of shaders the current list of shaders depends on
	Array<GLSubShader*> mAttached;	// Array of shaders currently attached to the program

	// Registered values
	mutable Array<UniformEntry> mUniforms;

	// Alternate versions of this shader
	mutable GLShader* mDeferred;	// Shader Name [Deferred]
	mutable GLShader* mShadowed;	// Shader Name [Shadowed]

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
	void SetUniform_LightParams		(const String& name, Uniform& uniform);

	void SetUniform_MatColor		(const String& name, Uniform& uniform);
	void SetUniform_MatParams0		(const String& name, Uniform& uniform);
	void SetUniform_MatParams1		(const String& name, Uniform& uniform);

	void SetUniform_MM				(const String& name, Uniform& uniform);
	void SetUniform_PM				(const String& name, Uniform& uniform);
	void SetUniform_MVM				(const String& name, Uniform& uniform);
	void SetUniform_MVPM			(const String& name, Uniform& uniform);
	void SetUniform_IVM				(const String& name, Uniform& uniform);
	void SetUniform_IVRM			(const String& name, Uniform& uniform);
	void SetUniform_IPM				(const String& name, Uniform& uniform);
	void SetUniform_IMVPM			(const String& name, Uniform& uniform);

	// Should only be accessible through GLGraphics
	bool Init (GLGraphics* graphics, const String& name);

	// Activates this shader
	bool Activate (bool resetUniforms);

	// Deactivates the current shader
	void Deactivate() const;

private:

	// INTERNAL: Appends the specified shader to the list
	void _Append (const String& filename);

	// INTERNAL: Detaches the attached shaders
	void _Detach();

	// INTERNAL: Releases all resources used by the shader
	void _Release();

	// INTERNAL: Validate the specified list of shaders
	bool _Validate (Array<GLSubShader*>& list);

	// INTERNAL: Attach all GLSL shaders to this shader program
	void _Attach (Array<GLSubShader*>& list);

	// INTERNAL: Link all shaders
	bool _Link();

	// INTERNAL: Updates registered uniforms bound to the shader
	void _UpdateUniforms();

	// INTERNAL: Updates a single uniform entry
	bool _UpdateUniform (uint glID, const Uniform& uni) const;

	// INTERNAL: Adds a new registered uniform value without checking to see if it already exists
	void _InsertUniform (const String& name, uint elements, const SetUniformDelegate& fnct);

public:

	GLShader() : mGraphics(0), mProgram(0), mIsDirty(false), mDeferred(0), mShadowed(0) {}
	virtual ~GLShader() {}

	// Clears the shader, making it invalid
	virtual void Clear() { mAdded.Clear(); mIsDirty = true; }

	// Marks the shader as needing to be relinked
	virtual void SetDirty() { mIsDirty = true; }

	// Adds the specified sub-shader to this program
	virtual void AddSubShader (ISubShader* sub);

	// Returns whether this shader program is using the specified shader
	virtual bool IsUsingSubShader (const ISubShader* sub) const;

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const;

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const;

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct);
};