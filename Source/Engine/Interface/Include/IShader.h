#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
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
			Skinned		= 0x1,	// The shader has a skinning component, has 'R5_boneTransforms' uniform
			Instanced	= 0x2,	// The shader has a pseudo-instancing component
			Billboarded	= 0x4,	// The shader is meant for a billboarded quad (or a series of quads)
		};
	};

	// Function used to set the uniform variable's data when activating the shader
	typedef FastDelegate<void (const String& name, Uniform& data)>	SetUniformDelegate;

protected:

	Flags	mFlags;
	String	mName;

public:

	// All shaders should have flags that can be easily modified
	bool GetFlag (uint flags) const			{ return mFlags.Get(flags); }

	// It should also be possible to change the flags
	void SetFlag (uint flags, bool value)	{ mFlags.Set(flags, value); }

public:

	R5_DECLARE_INTERFACE_CLASS("Shader");

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
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct)=0;
};