#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Unified shader interface
// Author: Michael Lyashenko
//============================================================================================================

struct IShader
{
	struct Type
	{
		enum
		{
			Unknown		= 0,
			Vertex		= 1,
			Fragment	= 2,
			Geometry	= 4,
			Surface		= 7,
		};
	};

	// Flags used by this class
	struct Flag
	{
		enum
		{
			LegacyFormat = 0x0001,	// This shader uses an outdated shader model (before SM 3.0)
			Vertex		 = 0x0002,	// This shader has a vertex component
			Fragment	 = 0x0004,	// This shader has a fragment component
			Surface		 = 0x0008,	// This is a surface shader, usable for both forward and deferred
			Skinned		 = 0x0010,	// The shader has a skinning component, has 'R5_boneTransforms' uniform
			Billboard	 = 0x0020,	// The shader has a skinning component, has 'R5_boneTransforms' uniform
			Shadowed	 = 0x0040,	// The material's last texture must be "R5_shadowMap"
			Fog			 = 0x0080,	// The shader will automatically add fog for forward rendering
			Deferred	 = 0x0100,	// Deferred rendering shader -- won't have any lighting
			DirLight	 = 0x0200,	// The shader expects a directional light
			PointLight	 = 0x0400,	// The shader expects a point light
			SpotLight	 = 0x0800,	// The shader expects a spot light
			Lit			 = 0x0E00,	// The shader expects some light (to be used as a convenience mask)
			DepthOnly	 = 0x1000,	// Only depth output is desired
		};
	};

	// Function used to set the uniform variable's data when activating the shader
	typedef FastDelegate<void (const String& name, Uniform& data)>	SetUniformDelegate;

protected:

	Flags	mDesired;
	Flags	mFinal;
	String	mName;
	uint	mUID;

	IShader() : mUID(GenerateUID()) {}

public:

	// All shaders should have flags that can be easily modified
	bool GetFlag (uint flags) const			{ return mFinal.Get(flags); }

	// It should also be possible to change the flags
	void SetFlag (uint flags, bool value)	{ mFinal.Set(flags, value); }

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

	// Sets the shader source code
	// NOTE: Setting a fragment shader followed by a vertex shader will not replace them, but will instead use both.
	virtual uint SetComponentCode (const String& code)=0;

	// Retrieves the source code of the specified shader component
	virtual const String& GetComponentCode (uint type) const=0;

	// Force-updates the value of the specified uniform
	virtual bool SetUniform (const String& name, const Uniform& uniform) const=0;

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint group = Uniform::Group::SetWhenActivated)=0;
};