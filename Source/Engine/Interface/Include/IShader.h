#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Unified shader interface
//============================================================================================================

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
			Processed	= 0x1,
			Skinned		= 0x2,
		};
	};

	// Function used to set the uniform variable's data when activating the shader
	typedef FastDelegate<void (const String& name, Uniform& data)>	SetUniformDelegate;

private:

	Flags mFlags;

public:

	// All shaders should have flags that can be easily modified
	inline Flags&		GetFlags()			{ return mFlags; }
	inline const Flags&	GetFlags() const	{ return mFlags; }

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
	// (Returns how many lights it was actually activated for)
	virtual uint Activate (uint activeLightCount, bool forceUpdateUniforms = false) const=0;

	// Deactivates the active shader
	virtual void Deactivate() const=0;

	// Force-updates the value of the specified uniform using either the existing function or the new one, if specified
	virtual bool UpdateUniform (uint id, const SetUniformDelegate& fnct = 0) const=0;

public:

	// Registers a uniform variable that will be updated when the shader is activated
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint id = 0)=0;

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