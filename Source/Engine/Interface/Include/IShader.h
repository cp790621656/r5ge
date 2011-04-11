#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Unified shader interface
//============================================================================================================

struct IShader
{
	// Flags used by this class
	struct Flag
	{
		enum
		{
			LegacyFormat = 0x0001,	// This shader uses an outdated shader model (before SM 3.0)
			Vertex		 = 0x0002,	// This shader has a vertex component
			Fragment	 = 0x0004,	// This shader has a fragment component
			Skinned		 = 0x0008,	// The shader has a skinning component, has 'R5_boneTransforms' uniform
			Shadowed	 = 0x0010,	// The material's last texture must be "R5_shadowMap"
			Surface		 = 0x0020,	// This is a surface shader, usable for both forward and deferred
		};
	};

	// Function used to set the uniform variable's data when activating the shader
	typedef FastDelegate<void (const String& name, Uniform& data)>	SetUniformDelegate;

protected:

	Flags	mFlags;
	String	mName;
	uint	mUID;

	IShader() : mUID(GenerateUID()) {}

public:

	// All shaders should have flags that can be easily modified
	bool GetFlag (uint flags) const			{ return mFlags.Get(flags); }

	// It should also be possible to change the flags
	void SetFlag (uint flags, bool value)	{ mFlags.Set(flags, value); }

public:

	R5_DECLARE_INTERFACE_CLASS("Shader");

	// Retrieves a unique identifier for this shader
	uint GetUID() const { return mUID; }

	virtual ~IShader() {}

	// All shaders should have unique names
	const String& GetName() const { return mName; }

	// Clears the shader, making it invalid
	virtual void Clear()=0;

	// Marks the shader as needing to be relinked
	virtual void SetDirty()=0;

	// Adds the specified sub-shader to this program
	virtual void AddSubShader (ISubShader* sub)=0;

	// Returns whether this shader program is using the specified shader
	virtual bool IsUsingSubShader (const ISubShader* sub) const=0;

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const=0;

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const=0;

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct, bool setOnDraw = false)=0;
};