#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Vertex and fragment shader management
//============================================================================================================

class GLShader : public IShader
{
public:

	struct UniformEntry
	{
		String						mName;		// Name of the uniform variable (ie: "R5_eyePos")
		mutable SetUniformDelegate	mDelegate;	// Delegate function that will set the data for that uniform
		mutable int					mGlID;		// OpenGL resource ID in the shader, once found (-2 if not checked, -1 if not found)

		UniformEntry() : mGlID(-2) {}
	};

	struct ShaderInfo
	{
		String	mPath;			// Path to the source of the shader
		String	mSource;		// Actual original source code
		bool	mSpecial;		// Whether it requires special pre-processing

		ShaderInfo() : mSpecial(false) {}

		void Clear()
		{
			mPath.Clear();
			mSource.Clear();
			mSpecial = false;
		}
	};
	
	struct ShaderEntry
	{
		struct CompileStatus
		{
			enum
			{
				Unknown	= 0,	// Not yet compiled
				Success	= 1,	// Compiled successfully at least once
				Error	= 2,	// Failed to compile, shader will not be used
			};
		};

		uint mGlID;		// OpenGL shader ID, once created
		byte mStatus;	// Compile status

		ShaderEntry() : mGlID(0), mStatus(CompileStatus::Unknown) {}
	};

private:

	struct ProgramEntry
	{
		ShaderEntry					mVertex;	// Compiled vertex shaders
		ShaderEntry					mFragment;	// Compiled fragment shaders
		ShaderEntry					mProgram;	// Linked shader programs
		mutable Array<UniformEntry>	mUniforms;	// Array of registered uniform variables for this program

		void RegisterUniform (const String& name, const SetUniformDelegate& fnct);
		bool UpdateUniform (const String& name, const Uniform& uniform) const;
		bool UpdateUniform (uint id, const SetUniformDelegate& fnct) const;
	};

private:

	String				mName;				// GLShader's name
	ShaderInfo			mVertexInfo;		// Vertex shader source code/path
	ShaderInfo			mFragmentInfo;		// Fragment shader source code/path
	Array<ProgramEntry>	mPrograms;			// List of programs, one per light
	bool				mIsDirty;			// Whether the shader should be rebuilt next frame
	bool				mSerializable;		// Whether the shader will be serialized
	uint				mLast;				// Starts with the maximum number of lights and moves down if shaders fail to compile
	ulong				mUpdateStamp;		// Last update timestamp
	Thread::Lockable	mLock;

public:

	GLShader(const String& name);
	~GLShader() { _InternalRelease(true); }

private:

	// INTERNAL: Non thread-safe version of Release()
	void _InternalRelease (bool clearUniforms);

	// INTERNAL: Non thread-safe version of Activate()
	ActivationResult _Activate (uint activeLighCount, bool updateUniforms);

	// INTERNAL: Sets the shader's source code, not thread safe
	void _SetSourceCode (const String& code, uint type);

	// INTERNAL: Sets the path where the shader's source code can be found
	void _SetSourcePath (const String& path, uint type);
	
public:

	// Retrieves the name of the shader
	virtual const String& GetName() const { return mName; }

	// Releases the source for the shader and marks it as dirty so it's released next frame
	virtual void Release();

	// Returns whether the shader is in a usable state
	virtual bool IsValid() const { return (mVertexInfo.mSource.IsValid() || mFragmentInfo.mSource.IsValid()); }

public: // The following functions are meant to be called only from the graphics thread

	// Activates the shader compiled for the specified number of lights
	virtual ActivationResult Activate (uint activeLightCount, bool forceUpdateUniforms = false) const;

	// Deactivates the active shader
	virtual void Deactivate() const;

	// Force-updates the value of the specified uniform
	virtual bool UpdateUniform (const String& name, const Uniform& uniform) const;

public:

	// Registers a uniform variable that's updated once per frame
	virtual void RegisterUniform (const String& name, const SetUniformDelegate& fnct);

	// Directly sets the source code for the shader
	virtual void SetSourceCode (const String& code, uint type) { mLock.Lock(); _SetSourceCode(code, type); mLock.Unlock(); }

	// Sets the path where the shader's source code can be found
	virtual void SetSourcePath (const String& path, uint type) { mLock.Lock(); _SetSourcePath(path, type); mLock.Unlock(); }

	// It's always useful to be able to retrieve what was once set
	virtual const String& GetSourcePath (uint type) const { return (type == Type::Vertex ? mVertexInfo.mPath   : mFragmentInfo.mPath);   }
	virtual const String& GetSourceCode (uint type) const { return (type == Type::Vertex ? mVertexInfo.mSource : mFragmentInfo.mSource); }
	
	// Serialization
	virtual bool IsSerializable() const { return mSerializable; }
	virtual void SetSerializable(bool val) { mSerializable = val; }
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	virtual bool SerializeTo (TreeNode& root) const;
};