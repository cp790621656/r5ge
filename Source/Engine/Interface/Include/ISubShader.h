#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Single vertex, fragment or geometry shader
//============================================================================================================

struct ISubShader
{
	struct Type
	{
		enum
		{
			Invalid		= 0,
			Vertex		= 1,
			Fragment	= 2,
			Geometry	= 3,
		};
	};

protected:

	Flags	mFlags;
	String	mName;

public:

	// All shaders should have flags that can be easily modified
	bool GetFlag (uint flags) const			{ return mFlags.Get(flags); }

	// It should also be possible to change the flags
	void SetFlag (uint flags, bool value)	{ mFlags.Set(flags, value); }

public:

	R5_DECLARE_INTERFACE_CLASS("SubShader");

	virtual ~ISubShader() {}

	// All shaders should have unique names
	const String& GetName() const { return mName; }

	// Retrieves the source code used to compile this shader
	virtual const String& GetCode() const=0;

	// Changes the code for the current shader and calls SetDirty()
	virtual void SetCode (const String& code, bool notifyShaders = true)=0;

	// Marks this shader and all shader programs using this shader as needing to be recompiled
	virtual void SetDirty()=0;

	// Validates the shader, compiling it if necessary
	virtual bool IsValid()=0;
};