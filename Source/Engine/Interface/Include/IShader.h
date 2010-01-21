#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Unified shader interface
//============================================================================================================

// TODO:
// - Graphics manager should have a list of ALL shaders with their compiled identifiers.
// - GLShader becomes a program that references existing shaders.
// - GLShader can reference one or more shaders. No cap on how many or of what type.
// - Shaders can reference other shaders from inside their code. This #include is parsed and "dependency"
//   entries are created, so when a shader A is included by GLShader, and it happens to be referencing
//   shader B, shader B gets added as well.
// - GetShader("Deferred/D") becomes:
//   1) GLShader is not found. Create a new entry.
//   2) GLShader automatically queries the Graphics system, asking to provide a shader "Deferred/D".
//   3) The graphics system doesn't find those entries, tries to load frag/vert versions of the files.
//   4) Shader files get loaded, entries get created. If any of them #include others, those files get
//      checked for as well and they too get included in the list of shaders.
//   5) Graphics returns a list of shaders back to GLShader.
//   6) GLShader creates an OpenGL shader program using the provided shaders.
// - Note that I need to change main() inside shaders to be VertexShader() / FragmentShader().

/*struct IShader
{
	R5_DECLARE_INTERFACE_CLASS("Shader");

	// Flags used by this class
	struct Flag
	{
		enum
		{
			Skinned		= 0x1,	// The shader has a skinning component, has 'R5_boneTransforms' uniform
			Instanced	= 0x2,	// The shader has a pseudo-instancing component
			Billboarded	= 0x4,	// The shader is meant for a billboarded quad (or a series of quads)
		};
	};

	// Function used to set the uniform variable's data when activating the shader
	typedef FastDelegate<void (const String& name, Uniform& data)>	SetUniformDelegate;

private:

	Flags mFlags;
	String mName;

public:

	// All shaders should have flags that can be easily modified
	bool GetFlag (uint flags) const			{ return mFlags.Get(flags); }

	// It should also be possible to change the flags
	void SetFlag (uint flags, bool value)	{ mFlags.Set(flags, value); }

public:

	virtual ~IShader() {};

	// Retrieves the name of the shader
	const String& GetName() const { return mName; }

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const=0;

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;
};*/

struct IShader
{
	R5_DECLARE_INTERFACE_CLASS("Shader");

	// Shader type -- vertex shader or fragment shader
	struct Type
	{
		enum
		{
			Vertex		= 0,
			Fragment	= 1,
		};
	};

	// Flags used by this class
	struct Flag
	{
		enum
		{
			Skinned		= 0x1,	// The shader has a skinning component, has 'R5_boneTransforms' uniform
			Instanced	= 0x2,	// The shader has a pseudo-instancing component
			Billboarded	= 0x4,	// The shader is meant for a billboarded quad (or a series of quads)
		};
	};

	// Struct returned by Activate()
	struct ActivationResult
	{
		uint mCount;	// How many lights the shader was activated for
		bool mReused;	// Whether the shader was reused or newly activated

		ActivationResult() : mCount(-1), mReused(true) {}
	};

	// Function used to set the uniform variable's data when activating the shader
	typedef FastDelegate<void (const String& name, Uniform& data)>	SetUniformDelegate;

private:

	Flags mFlags;

public:

	// All shaders should have flags that can be easily modified
	bool GetFlag (uint flags) const			{ return mFlags.Get(flags); }

	// It should also be possible to change the flags
	void SetFlag (uint flags, bool value)	{ mFlags.Set(flags, value); }

public:

	virtual ~IShader() {};

	// Retrieves the name of the shader
	virtual const String& GetName() const=0;

	// Releases all resources currently taken up by the shader
	virtual void Release()=0;

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const=0;

public: // The following functions are meant to be called only from the graphics thread

	// Activates the shader compiled for the specified number of lights
	virtual ActivationResult Activate (uint activeLightCount, bool forceUpdateUniforms = false) const=0;

	// Deactivates the active shader
	virtual void Deactivate() const=0;

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const=0;

public:

	// Registers a uniform variable that will be updated when the shader is activated
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct)=0;

	// Directly sets the source code for the shader
	virtual void SetSourceCode (const String& code, uint type)=0;

	// Sets the path where the shader's source code can be found
	virtual void SetSourcePath (const String& path, uint type)=0;

	// It's always useful to be able to retrieve what was once set
	virtual const String&	GetSourcePath (uint type) const=0;
	virtual const String&	GetSourceCode (uint type) const=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;
};