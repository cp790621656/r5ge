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
// Shader callback function for R5_time uniform
//------------------------------------------------------------------------------------------------------------
// R5_time.x = Current time in seconds
// R5_time.y = Irregular wavy sin(time), used for wind
// R5_time.z = sin(R5_time.z) gives a 360 degree rotation every 1000 seconds
//============================================================================================================

void SetUniform_Time (const String& name, Uniform& uniform)
{
	uniform.mType = Uniform::Type::Float3;
	double time = Time::GetSeconds();
	uniform.mVal[0] = (float)time;
	uniform.mVal[1] = (float)((0.6 * sin(time * 0.421) +
					   0.3 * sin(time * 1.737) +
					   0.1 * cos(time * 2.786)) * 0.5 + 0.5);

	double temp = time * 0.001;
	uniform.mVal[2] = (float)((temp - floor(temp)) * TWOPI);
}

//============================================================================================================
// Shader callback function for R5_eyePos
//============================================================================================================

void GLShader::SetUniform_EyePos (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetCameraPosition();
}

//============================================================================================================
// Shader callback function for R5_pixelSize
//------------------------------------------------------------------------------------------------------------
// Can be used to figure out 0-1 range full-screen texture coordinates in the fragment shader:
// gl_FragCoord.xy * R5_pixelSize
//============================================================================================================

void GLShader::SetUniform_PixelSize (const String& name, Uniform& uniform)
{
	const Vector2f& size ( mGraphics->GetActiveViewport() );
	uniform.mType = Uniform::Type::Float2;
	uniform.mVal[0] = 1.0f / size.x;
	uniform.mVal[1] = 1.0f / size.y;
}

//============================================================================================================
// Shader callback function for R5_clipRange
//------------------------------------------------------------------------------------------------------------
// R5_clipRange.x = near
// R5_clipRange.y = far
// R5_clipRange.z = near * far
// R5_clipRange.w = far - near
//------------------------------------------------------------------------------------------------------------
// Formula used to calculate fragment's linear depth:
//------------------------------------------------------------------------------------------------------------
// (R5_clipRange.z / (R5_clipRange.y - gl_FragCoord.z * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;
//============================================================================================================

void GLShader::SetUniform_ClipRange (const String& name, Uniform& uniform)
{
	const Vector3f& range = mGraphics->GetCameraRange();
	uniform = Quaternion(range.x,
						 range.y,
						 range.x * range.y,
						 range.y - range.x);
}

//============================================================================================================
// Shader callback function for R5_fogRange
//============================================================================================================

void GLShader::SetUniform_FogRange (const String& name, Uniform& uniform)
{
	Vector2f range (mGraphics->GetFogRange());

	if (range.x > 1.0001f || range.y > 1.0001f)
	{
		// Absolute values were used -- convert them to 0 to 1 range
		const Vector3f& camRange = mGraphics->GetCameraRange();
		float dist = camRange.y - camRange.x;
		range.Set((range.x - camRange.x) / dist, (range.y - camRange.x) / dist);
	}

	// R5_fogRange expects a relative-to-start distance in the 2nd parameter
	range.y -= range.x;
	uniform = range;
}

//============================================================================================================
// Shader callback for R5_projectionMatrix
//============================================================================================================

void GLShader::SetUniform_PM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetProjectionMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseViewMatrix
//============================================================================================================

void GLShader::SetUniform_IVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseModelViewMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseProjMatrix
//============================================================================================================

void GLShader::SetUniform_IPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseProjMatrix();
}

//============================================================================================================
// Shader callback function for R5_inverseViewRotationMatrix
//============================================================================================================

void GLShader::SetUniform_IVRM (const String& name, Uniform& uniform)
{
	const Matrix43& mv = mGraphics->GetModelViewMatrix();
	uniform.mType = Uniform::Type::Float9;
	uniform.mVal[0] = mv[0];
	uniform.mVal[1] = mv[4];
	uniform.mVal[2] = mv[8];
	uniform.mVal[3] = mv[1];
	uniform.mVal[4] = mv[5];
	uniform.mVal[5] = mv[9];
	uniform.mVal[6] = mv[2];
	uniform.mVal[7] = mv[6];
	uniform.mVal[8] = mv[10];
}

//============================================================================================================
// Shader callback function for R5_worldTransformMatrix
//============================================================================================================

void GLShader::SetUniform_WTM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix();
}

//============================================================================================================
// Shader callback function for R5_worldRotationMatrix
//============================================================================================================

void GLShader::SetUniform_WRM (const String& name, Uniform& uniform)
{
	const Matrix43& model = mGraphics->GetModelMatrix();
	uniform.mType = Uniform::Type::Float9;
	uniform.mVal[0] = model[0];
	uniform.mVal[1] = model[1];
	uniform.mVal[2] = model[2];
	uniform.mVal[3] = model[4];
	uniform.mVal[4] = model[5];
	uniform.mVal[5] = model[6];
	uniform.mVal[6] = model[8];
	uniform.mVal[7] = model[9];
	uniform.mVal[8] = model[10];
}

//============================================================================================================
// Initialize the shader
//============================================================================================================

bool GLShader::Init (GLGraphics* graphics, const String& name)
{
	mGraphics	= graphics;
	mName		= name;

	// Shaders that begin with [R5] are built-in shaders
	if (name.BeginsWith("[R5]") || System::FileExists(name))
	{
		// Exact match -- use this shader
		_Append(name);
	}
	else
	{
		// No exact match -- try to find the common shader types
		_Append(::FindShader(name, ".vert"));
		_Append(::FindShader(name, ".frag"));
		_Append(::FindShader(name, ".geom"));
	}

	// Register common uniforms that remain identical in all shaders
	_InsertUniform( "R5_time",						&SetUniform_Time );
	_InsertUniform( "R5_worldEyePosition",			bind(&GLShader::SetUniform_EyePos,		this) );
	_InsertUniform( "R5_pixelSize",					bind(&GLShader::SetUniform_PixelSize,	this) );
	_InsertUniform( "R5_clipRange",					bind(&GLShader::SetUniform_ClipRange,	this) );
	_InsertUniform( "R5_fogRange",					bind(&GLShader::SetUniform_FogRange,	this) );
	_InsertUniform( "R5_projectionMatrix",			bind(&GLShader::SetUniform_PM,			this) );
	_InsertUniform( "R5_inverseViewMatrix",			bind(&GLShader::SetUniform_IVM,			this) );
	_InsertUniform( "R5_inverseProjMatrix",			bind(&GLShader::SetUniform_IPM,			this) );
	_InsertUniform( "R5_inverseViewRotationMatrix",	bind(&GLShader::SetUniform_IVRM,		this) );
	_InsertUniform( "R5_worldTransformMatrix",		bind(&GLShader::SetUniform_WTM,			this) );
	_InsertUniform( "R5_worldRotationMatrix",		bind(&GLShader::SetUniform_WRM,			this) );

	// Whether the shader is actually already valid or not depends on whether it has any sub-shaders
	return mAdded.IsValid();
}

//============================================================================================================
// Only GLGraphics should be activating shaders
//============================================================================================================

bool GLShader::Activate (bool resetUniforms)
{
	if (mIsDirty)
	{
		mIsDirty = false;
		_Detach();

		if (mAdded.IsValid())
		{
			for (uint i = mAdded.GetSize(); i > 0; )
			{
				GLSubShader* sub = mAdded[--i];
				sub->AppendDependenciesTo(mDepended);
			}
			return _Link();
		}
	}

	if (mAdded.IsEmpty())
	{
		if (g_activeProgram != 0) glUseProgram(g_activeProgram = 0);
		return false;
	}

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
	return _Link();
}

//============================================================================================================
// Deactivates the current shader
//============================================================================================================

void GLShader::Deactivate() const
{
	if (g_activeProgram != 0) glUseProgram(g_activeProgram = 0);
}

//============================================================================================================
// INTERNAL: Appends the specified shader to the list
//============================================================================================================

void GLShader::_Append (const String& filename)
{
	if (filename.IsValid())
	{
		GLSubShader* sub = mGraphics->GetGLSubShader(filename, true, ISubShader::Type::Invalid);
		mAdded.AddUnique(sub);
		mIsDirty = true;
	}
}

//============================================================================================================
// INTERNAL: Detaches the attached shaders
//============================================================================================================

void GLShader::_Detach()
{
	for (uint i = mAttached.GetSize(); i > 0; )
	{
		GLSubShader* sub = mAttached[--i];

		if (sub->mGLID != -1)
		{
			glDetachShader(mProgram, sub->mGLID);
			CHECK_GL_ERROR;
		}
	}
	mAttached.Clear();
	mDepended.Clear();
}

//============================================================================================================
// INTERNAL: Releases all resources used by the shader
//============================================================================================================

void GLShader::_Release()
{
	mIsDirty = false;

	if (mProgram != 0)
	{
		// If this program is currently active, deactivate it
		if (g_activeProgram == mProgram)
		{
			glUseProgram(g_activeProgram = 0);
		}

		// Detach all attached shaders
		_Detach();

		// Delete this GLSL program
		glDeleteProgram(mProgram);
		CHECK_GL_ERROR;
		mProgram = 0;
	}
}

//============================================================================================================
// INTERNAL: Validate the specified list of shaders
//============================================================================================================

bool GLShader::_Validate (Array<GLSubShader*>& list)
{
	for (uint i = list.GetSize(); i > 0; )
	{
		GLSubShader* sub = list[--i];

		if (!sub->IsValid())
		{
			ASSERT(false, "Attempting to use a missing shader!");
			Deactivate();
			return false;
		}
	}
	return true;
}

//============================================================================================================
// INTERNAL: Attach all GLSL shaders to this shader program
//============================================================================================================

void GLShader::_Attach (Array<GLSubShader*>& list)
{
	for (uint i = list.GetSize(); i > 0; )
	{
		GLSubShader* sub = list[--i];
		glAttachShader(mProgram, sub->mGLID);
		mFlags.Include(sub->mFlags);
		mAttached.Expand() = sub;
	}
}

//============================================================================================================
// Link all shaders
//============================================================================================================

bool GLShader::_Link()
{
	mIsDirty = false;

	// Ensure that both shader lists pass validation
	if (!_Validate(mAdded) || !_Validate(mDepended)) return false;

	// Create the GLSL program
	if (mProgram == 0)
	{
		mProgram = glCreateProgram();
		ASSERT( mProgram != 0, glGetErrorString() );
		CHECK_GL_ERROR;
	}
	else
	{
		// Detach all currently attached shaders
		_Detach();
	}

	// Attach all shaders to this shader program
	_Attach(mAdded);
	_Attach(mDepended);
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
		for (uint i = 0; i < mAttached.GetSize(); ++i)
		{
			GLSubShader* sub = mAttached[i];
			const char* type = (sub->mType == GLSubShader::Type::Vertex) ? "Vertex" : "Fragment";
			if (sub->mType == GLSubShader::Type::Geometry) type = "Geometry";
			System::Log("          - Using '%s' (%s)", sub->mName.GetBuffer(), type);
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
			// Print the debug log if there is something to print
			Array<String> lines;
			R5::CreateDebugLog(lines, log, "");

			if (lines.IsValid())
			{
				FOREACH(i, lines)
				{
					System::Log("          - %s", lines[i].GetBuffer());
				}
				System::FlushLog();
			}

#ifdef _DEBUG
			String errMsg ("Failed to link '");
			errMsg << mName;
			errMsg << "'!";

			FOREACH(i, lines)
			{
				errMsg << "\n\n";
				errMsg << lines[i];
			}
			ASSERT(false, errMsg.GetBuffer());
#endif
			// Release this shader
			_Release();
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
// INTERNAL: Adds a new registered uniform value without checking to see if it already exists
//============================================================================================================

void GLShader::_InsertUniform (const String& name, const SetUniformDelegate& fnct)
{
	UniformEntry& entry = mUniforms.Expand();
	entry.mName			= name;
	entry.mDelegate		= fnct;
}

//============================================================================================================
// Adds the specified sub-shader to this program
//============================================================================================================

void GLShader::AddSubShader (ISubShader* sub)
{
	GLSubShader* gls = (GLSubShader*)sub;
	if (mAdded.AddUnique(gls)) mIsDirty = true;
}

//============================================================================================================
// Returns whether this shader program is using the specified shader
//============================================================================================================

bool GLShader::IsUsingSubShader (const ISubShader* sub) const
{
	GLSubShader* gls = (GLSubShader*)sub;
	return mAttached.Contains(gls) || mDepended.Contains(gls);
}

//============================================================================================================
// Returns whether the shader is in a usable state
//============================================================================================================

bool GLShader::IsValid() const
{
	if (!mAdded.IsValid()) return false;

	FOREACH(i, mAdded)
	{
		ISubShader* sub = mAdded[i];
		if (!sub->IsValid()) return false;
	}
	return true;
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
			// The uniform has not yet been located
			if (entry.mGLID == -2) entry.mGLID = ::GetUniformID(name);

			// The uniform does not exist
			if (entry.mGLID == -1) return false;

			// The uniform exists and can be updated
			_UpdateUniform(entry.mGLID, uniform);
			return true;
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
	_InsertUniform(name, fnct);
}