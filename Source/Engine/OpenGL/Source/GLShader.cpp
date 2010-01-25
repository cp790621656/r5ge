#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Keep track of the currently active program
//============================================================================================================

uint g_activeProgram = 0;

//============================================================================================================
// Finds a shader file given the filename and extension
//============================================================================================================

String FindShader (const String& file, const char* extension)
{
	String path (file);
	path << extension;
	if (System::FileExists(path)) return path;

	if (!path.BeginsWith("Shaders/"))
	{
		path = String("Shaders/") + path;
		if (System::FileExists(path)) return path;
	}

	path = String("Shaders/");
	path << System::GetFilenameFromPath(file);
	if (System::FileExists(path)) return path;

	path << extension;
	if (System::FileExists(path)) return path;

	return "";
}

//============================================================================================================
// Sets a uniform integer value in a shader
//============================================================================================================

bool SetUniform1i (uint program, const char* name, int val)
{
	if (program)
	{
		int loc = glGetUniformLocation(program, name);

		if (loc != -1)
		{
#ifdef _DEBUG
			System::Log("          - Found uniform '%s' [%u]", name, loc);
#endif
			glUniform1i(loc, val);
			CHECK_GL_ERROR;
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Find the OpenGL ID of the specified uniform entry
//============================================================================================================

uint GetUniformID (const String& name)
{
	uint glID = glGetUniformLocation(g_activeProgram, name.GetBuffer());
	CHECK_GL_ERROR;

#ifdef _DEBUG
	if (glID != -1) System::Log("          - Found uniform '%s' [%u]", name.GetBuffer(), glID);
#endif
	return glID;
}

//============================================================================================================
// Deleting the shader should also detach all sub-shaders and delete the GLSL program
//============================================================================================================

GLShader::~GLShader()
{
	if (mProgram != 0)
	{
		for (uint i = mSubShaders.GetSize(); i > 0; )
		{
			GLSubShader* sub = mSubShaders[--i];

			if (sub->mGLID != -1)
			{
				glDetachShader(mProgram, sub->mGLID);
			}
		}
		glDeleteProgram(mProgram);
		mProgram = 0;
	}
}

//============================================================================================================
// Initialize the shader
//============================================================================================================

bool GLShader::Init (GLGraphics* graphics, const String& name)
{
	mGraphics	= graphics;
	mName		= name;

	if (System::FileExists(name))
	{
		// Exact match -- use this shader
		GLSubShader* sub = mGraphics->GetSubShader(name);
		mSubShaders.Expand() = sub;
		sub->AppendDependenciesTo(mSubShaders);
	}
	else
	{
		// No exact match -- try to find the common shader types
		String vert	(::FindShader(name, ".vert"));
		String frag (::FindShader(name, ".frag"));
		String geom (::FindShader(name, ".geom"));

		if (vert.IsValid())
		{
			GLSubShader* sub = mGraphics->GetSubShader(vert);
			mSubShaders.Expand() = sub;
			sub->AppendDependenciesTo(mSubShaders);
		}

		if (frag.IsValid() && frag != vert)
		{
			GLSubShader* sub = mGraphics->GetSubShader(frag);
			mSubShaders.Expand() = sub;
			sub->AppendDependenciesTo(mSubShaders);
		}

		if (geom.IsValid() && geom != vert && geom != frag)
		{
			GLSubShader* sub = mGraphics->GetSubShader(geom);
			mSubShaders.Expand() = sub;
			sub->AppendDependenciesTo(mSubShaders);
		}
	}
	return mSubShaders.IsValid();
}

//============================================================================================================
// Only GLGraphics should be activating shaders
//============================================================================================================

bool GLShader::Activate (bool resetUniforms)
{
	if (mProgram != 0)
	{
		if (g_activeProgram != mProgram)
		{
			glUseProgram(g_activeProgram = mProgram);
			_UpdateUniforms();
			return true;
		}
		else if (resetUniforms)
		{
			_UpdateUniforms();
		}
		return false;
	}
	else if (mSubShaders.IsEmpty())
	{
		if (g_activeProgram != 0)
		{
			glUseProgram(g_activeProgram = 0);
		}
		return false;
	}
	return _Link();
}

//============================================================================================================
// Deactivates the current shader
//============================================================================================================

void GLShader::Deactivate() const
{
	if (g_activeProgram != 0)
	{
		glUseProgram(g_activeProgram = 0);
	}
}

//============================================================================================================
// Link all shaders
//============================================================================================================

bool GLShader::_Link()
{
	// Ensure that all the sub-shaders are present first
	for (uint i = mSubShaders.GetSize(); i > 0; )
	{
		GLSubShader* sub = mSubShaders[--i];

		if (!sub->IsValid())
		{
			ASSERT(false, "Attempting to use a missing shader!");
			Deactivate();
			return false;
		}
	}

	// Create the GLSL program
	mProgram = glCreateProgram();
	ASSERT( mProgram != 0, glGetErrorString() );
	CHECK_GL_ERROR;

	// Attach all GLSL shaders to this shader program
	for (uint i = mSubShaders.GetSize(); i > 0; )
	{
		GLSubShader* sub = mSubShaders[--i];
		glAttachShader(mProgram, sub->mGLID);
		mFlags.Include(sub->mFlags);
	}
	CHECK_GL_ERROR;

	// Bind all one-time attributes prior to linking the program
	glBindAttribLocation(mProgram, IGraphics::Attribute::Position,		"R5_position");
	glBindAttribLocation(mProgram, IGraphics::Attribute::Tangent,		"R5_tangent");
	glBindAttribLocation(mProgram, IGraphics::Attribute::Normal,		"R5_normal");
	glBindAttribLocation(mProgram, IGraphics::Attribute::Color,			"R5_color");
	glBindAttribLocation(mProgram, IGraphics::Attribute::SecondaryColor,"R5_secondaryColor");
	glBindAttribLocation(mProgram, IGraphics::Attribute::FogCoord,		"R5_fogCoord");
	glBindAttribLocation(mProgram, IGraphics::Attribute::BoneWeight,	"R5_boneWeight");
	glBindAttribLocation(mProgram, IGraphics::Attribute::BoneIndex,		"R5_boneIndex");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord0,		"R5_texCoord0");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord1,		"R5_texCoord1");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord2,		"R5_texCoord2");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord3,		"R5_texCoord3");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord4,		"R5_texCoord4");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord5,		"R5_texCoord5");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord6,		"R5_texCoord6");
	glBindAttribLocation(mProgram, IGraphics::Attribute::TexCoord7,		"R5_texCoord7");
	CHECK_GL_ERROR;

	// Link the program
	glLinkProgram(mProgram);
	CHECK_GL_ERROR;

	// Get the linking status
	int retVal (0);
	glGetProgramiv(mProgram, GL_LINK_STATUS, &retVal);
	
#ifndef _DEBUG
	if (retVal != GL_TRUE)
#endif
	{
		String log;
		int logLength (0);
		glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 0)
		{
			log.Resize(logLength);
			int charsWritten (0);
			glGetProgramInfoLog (mProgram, logLength, &charsWritten, (char*)log.GetBuffer());
		}

		if (retVal == GL_TRUE)
		{
			System::Log("[SHADER]  '%s' has linked successfully", mName.GetBuffer());
		}
		else
		{
			System::Log("[SHADER]  '%s' has FAILED to link!", mName.GetBuffer());
		}

		// List all the shaders used by this GLSL program
		for (uint i = 0; i < mSubShaders.GetSize(); ++i)
		{
			GLSubShader* sub = mSubShaders[i];
			const char* type = (sub->mType == GLSubShader::Type::Vertex) ? "Vertex" : "Fragment";
			if (sub->mType == GLSubShader::Type::Geometry) type = "Geometry";
			System::Log("          - Using '%s' (%s)", sub->mSource.GetBuffer(), type);
		}

		if (retVal == GL_TRUE)
		{
			// List the program's common supported features
			if (mFlags.Get(IShader::Flag::Billboarded)) System::Log("          - Supports billboarding");
			if (mFlags.Get(IShader::Flag::Instanced))	System::Log("          - Supports instancing");
			if (mFlags.Get(IShader::Flag::Skinned))		System::Log("          - Supports skinning");
		}
		else
		{
			R5::PrintDebugLog(log);
#ifdef _DEBUG
			String errMsg ("Failed to link '");
			errMsg << mName;
			errMsg << "'!";
			ASSERT(false, errMsg.GetBuffer());
#endif
			// Delete the shader and release the list of sub-shaders so it's not created again
			glDeleteProgram(mProgram);
			mProgram = 0;
			mSubShaders.Clear();
			CHECK_GL_ERROR;
			return false;
		}
	}

	// Use this program
	glUseProgram(g_activeProgram = mProgram);
	CHECK_GL_ERROR;

	// Set the constant values of texture units in the shader
	::SetUniform1i(mProgram, "R5_texture0", 0);
	::SetUniform1i(mProgram, "R5_texture1", 1);
	::SetUniform1i(mProgram, "R5_texture2", 2);
	::SetUniform1i(mProgram, "R5_texture3", 3);
	::SetUniform1i(mProgram, "R5_texture4", 4);
	::SetUniform1i(mProgram, "R5_texture5", 5);
	::SetUniform1i(mProgram, "R5_texture6", 6);
	::SetUniform1i(mProgram, "R5_texture7", 7);
	CHECK_GL_ERROR;

	// Update the uniforms
	_UpdateUniforms();
	return true;
}

//============================================================================================================
// INTERNAL: Updates registered uniforms bound to the shader
//============================================================================================================

void GLShader::_UpdateUniforms()
{
	Uniform uni;

	for (uint u = mUniforms.GetSize(); u > 0; )
	{
		UniformEntry& entry = mUniforms[--u];

		if (entry.mGLID != -1 && entry.mDelegate)
		{
			// Find the uniform if we have not yet tried to find it
			if (entry.mGLID == -2) entry.mGLID = ::GetUniformID(entry.mName);

			// If the uniform has been found, update it
			if (entry.mGLID != -1)
			{
				uni.mType = Uniform::Type::Invalid;
				entry.mDelegate(entry.mName, uni);
				_UpdateUniform(entry.mGLID, uni);
			}
		}
	}
}

//============================================================================================================
// INTERNAL: Updates a single uniform entry
//============================================================================================================

bool GLShader::_UpdateUniform (uint glID, const Uniform& uni) const
{
	if (glID >= 0)
	{
		switch (uni.mType)
		{
		case Uniform::Type::Float1:
			glUniform1f(glID, uni.mVal[0]);
			break;
		case Uniform::Type::Float2:
			glUniform2f(glID, uni.mVal[0], uni.mVal[1]);
			break;
		case Uniform::Type::Float3:
			glUniform3f(glID, uni.mVal[0], uni.mVal[1], uni.mVal[2]);
			break;
		case Uniform::Type::Float4:
			glUniform4f(glID, uni.mVal[0], uni.mVal[1], uni.mVal[2], uni.mVal[3]);
			break;
		case Uniform::Type::Float9:
			glUniformMatrix3fv(glID, 1, 0, uni.mVal);
			break;
		case Uniform::Type::Float16:
			glUniformMatrix4fv(glID, 1, 0, uni.mVal);
			break;
		case Uniform::Type::ArrayFloat1:
			glUniform1fv(glID, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat2:
			glUniform2fv(glID, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat3:
			glUniform3fv(glID, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat4:
			glUniform4fv(glID, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat9:
			glUniformMatrix3fv(glID, uni.mElements, 0, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat16:
			glUniformMatrix4fv(glID, uni.mElements, 0, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayInt:
			glUniform1iv(glID, uni.mElements, (int*)uni.mPtr);
			break;
		}
		CHECK_GL_ERROR;
		return true;
	}
	return false;
}

//============================================================================================================
// Force-updates the value of the specified uniform
//============================================================================================================

bool GLShader::SetUniform (const String& name, const Uniform& uniform) const
{
	ASSERT(g_activeProgram == mProgram, "Setting a uniform on an incorrect program?");

	for (uint i = mUniforms.GetSize(); i > 0; )
	{
		UniformEntry& entry = mUniforms[--i];

		if (entry.mName == name)
		{
			if (entry.mGLID == -2) entry.mGLID = ::GetUniformID(name);
			if (entry.mGLID != -1)
			{
				_UpdateUniform(entry.mGLID, uniform);
				return true;
			}
		}
	}

	// This must be a new entry -- try to add it
	{
		UniformEntry& entry = mUniforms.Expand();
		entry.mName = name;
		entry.mGLID = ::GetUniformID(name);
		
		if (entry.mGLID != -1)
		{
			_UpdateUniform(entry.mGLID, uniform);
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Registers a uniform variable that's updated once per frame
//============================================================================================================

void GLShader::RegisterUniform (const String& name, const SetUniformDelegate& fnct)
{
	for (uint i = 0; i < mUniforms.GetSize(); ++i)
	{
		UniformEntry& entry = mUniforms[i];

		if (entry.mName == name)
		{
			entry.mDelegate = fnct;
			return;
		}
	}

	UniformEntry& entry = mUniforms.Expand();
	entry.mName			= name;
	entry.mDelegate		= fnct;
}