#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Keep track of the currently active program
//============================================================================================================

uint g_activeProgram = 0;

//============================================================================================================
// For the sake of not duplicating data, all R5 built-ins are stored in a library-accessible list.
//============================================================================================================

bool g_fillUniformList = true;
Array<GLShader::UniformRecord> g_uniforms;

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
	uniform = Quaternion(range.x, range.y, range.x * range.y, range.y - range.x);
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
// Shader callback function for R5_fogColor
//============================================================================================================

void GLShader::SetUniform_FogColor (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetBackgroundColor();
}

//============================================================================================================
// Shader callback for R5_modelScale
//============================================================================================================

void GLShader::SetUniform_MS (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix().GetScale();
}

//============================================================================================================
// Shader callback function for R5_modelMatrix
//============================================================================================================

void GLShader::SetUniform_MM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix();
}

//============================================================================================================
// Shader callback for R5_projectionMatrix
//============================================================================================================

void GLShader::SetUniform_PM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetProjectionMatrix();
}

//============================================================================================================
// Shader callback function for R5_modelViewMatrix
//============================================================================================================

void GLShader::SetUniform_MVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelViewMatrix();
}

//============================================================================================================
// Shader callback for R5_modelViewProjMatrix
//============================================================================================================

void GLShader::SetUniform_MVPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelViewProjMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseViewMatrix
//============================================================================================================

void GLShader::SetUniform_IVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseModelViewMatrix();
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
// Shader callback for R5_inverseProjMatrix
//============================================================================================================

void GLShader::SetUniform_IPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseProjMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseMVPMatrix
//============================================================================================================

void GLShader::SetUniform_IMVPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseMVPMatrix();
}

//============================================================================================================
// Shader callback function for R5_lightAmbient
//============================================================================================================

void GLShader::SetUniform_LightAmbient (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mAmbient;
	uniform.mType = Uniform::Type::Float3;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShader::SetUniform_LightDiffuse (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mDiffuse;
	uniform.mType = Uniform::Type::Float3;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShader::SetUniform_LightPosition (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);

	Vector4f pos;

	if (light.mType == ILight::Type::Directional)
	{
		pos.xyz() = -light.mDir % mGraphics->GetModelViewMatrix();
		pos.w = 0.0f;
	}
	else
	{
		pos.xyz() = light.mPos * mGraphics->GetModelViewMatrix();
		pos.w = 1.0f;
	}
	uniform = pos;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShader::SetUniform_LightParams (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mParams;
}

//============================================================================================================
// Shader callback function for R5_materialColor
//============================================================================================================

void GLShader::SetUniform_MatColor (const String& name, Uniform& uniform)
{
	const IMaterial* mat = mGraphics->GetActiveMaterial();

	if (mat == 0 || mat == ((void*)-1))
	{
		uniform = Color4f(1.0f);
	}
	else
	{
		uniform = mat->GetDiffuse();
	}
}

//============================================================================================================
// Shader callback function for R5_materialParams0
//============================================================================================================

void GLShader::SetUniform_MatParams0 (const String& name, Uniform& uniform)
{
	const IMaterial* mat = mGraphics->GetActiveMaterial();

	if (mat == 0 || mat == ((void*)-1))
	{
		uniform = Vector4f(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		uniform = Vector4f(
			mat->GetSpecularity(),
			mat->GetSpecularHue(),
			mat->GetGlow(),
			mat->GetOcclusion());
	}
}

//============================================================================================================
// Shader callback function for R5_materialParams1
//============================================================================================================

void GLShader::SetUniform_MatParams1 (const String& name, Uniform& uniform)
{
	const IMaterial* mat = mGraphics->GetActiveMaterial();

	if (mat == 0 || mat == ((void*)-1))
	{
		uniform = Vector2f(0.25f, 0.0f);
	}
	else
	{
		uniform = Vector2f(mat->GetShininess(), mat->GetReflectiveness());
	}
}

//============================================================================================================
// Initialize the shader
//============================================================================================================

bool GLShader::Init (GLGraphics* graphics, const String& name)
{
	mGraphics	= graphics;
	mName		= name;

	// These uniforms will be set on IShader::Activate()
	_InsertUniform( "R5_time",				3,  &SetUniform_Time, false );
	_InsertUniform( "R5_eyePosition",		3,  bind(&GLShader::SetUniform_EyePos,			this), false );
	_InsertUniform( "R5_pixelSize",			2,  bind(&GLShader::SetUniform_PixelSize,		this), false );
	_InsertUniform( "R5_clipRange",			4,  bind(&GLShader::SetUniform_ClipRange,		this), false );
	_InsertUniform( "R5_fogRange",			2,  bind(&GLShader::SetUniform_FogRange,		this), false );
	_InsertUniform( "R5_fogColor",			4,  bind(&GLShader::SetUniform_FogColor,		this), false );
	_InsertUniform( "R5_lightAmbient",		3,  bind(&GLShader::SetUniform_LightAmbient,	this), false );
	_InsertUniform( "R5_lightDiffuse",		3,  bind(&GLShader::SetUniform_LightDiffuse,	this), false );
	_InsertUniform( "R5_lightPosition",		4,  bind(&GLShader::SetUniform_LightPosition,	this), false );
	_InsertUniform( "R5_lightParams",		3,  bind(&GLShader::SetUniform_LightParams,		this), false );
	_InsertUniform( "R5_materialColor",		4,  bind(&GLShader::SetUniform_MatColor,		this), false );
	_InsertUniform( "R5_materialParams0",	4,  bind(&GLShader::SetUniform_MatParams0,		this), false );
	_InsertUniform( "R5_materialParams1",	2,  bind(&GLShader::SetUniform_MatParams1,		this), false );

	// These uniforms will be set on IShader::Update(), which happens just before the drawing operations
	_InsertUniform( "R5_modelScale",				3,  bind(&GLShader::SetUniform_MS,		this), true );
	_InsertUniform( "R5_modelMatrix",				16, bind(&GLShader::SetUniform_MM,		this), true );
	_InsertUniform( "R5_projectionMatrix",			16, bind(&GLShader::SetUniform_PM,		this), true );
	_InsertUniform( "R5_modelViewMatrix",			16, bind(&GLShader::SetUniform_MVM,		this), true );
	_InsertUniform( "R5_modelViewProjMatrix",		16, bind(&GLShader::SetUniform_MVPM,	this), true );
	_InsertUniform( "R5_inverseViewMatrix",			16, bind(&GLShader::SetUniform_IVM,		this), true );
	_InsertUniform( "R5_inverseProjMatrix",			16, bind(&GLShader::SetUniform_IPM,		this), true );
	_InsertUniform( "R5_inverseMVPMatrix",			16, bind(&GLShader::SetUniform_IMVPM,	this), true );
	_InsertUniform( "R5_inverseViewRotationMatrix",	9,  bind(&GLShader::SetUniform_IVRM,	this), true );

	// The list of R5 uniforms should now be complete
	g_fillUniformList = false;

	// Shaders that begin with [R5] are built-in shaders
	if (name.BeginsWith("[R5]"))
	{
		// Exact match -- use this shader
		_Append(name);
	}
	else
	{
		// Clean up the filename, removing parameters specified in square brackets
		String path (System::GetPathFromFilename(name));
		path << System::GetFilenameFromPath(name, true);

		// Collect the files matching the specified path
		Array<String> files;
		System::GetFiles(path, files, true);
		System::GetFiles("Shaders/" + path, files, true);

		// This is the expected filename
		String expectedName (System::GetFilenameFromPath(name, false));

		// Shader parameters are specified after a bracket (this part is trimmed by GetFilenameFromPath)
		uint bracket = name.Find("[");
		String file, arguments;
		if (bracket < name.GetSize()) name.GetString(arguments, bracket);

		// Run through all located files and use shaders with filenames matching the expected value
		FOREACH(i, files)
		{
			file = files[i];
			String fileName (System::GetFilenameFromPath(file, false));

			// Shader file names must match exactly (ie: Deferred/D shouldn't match Deferred/D_N)
			if (fileName == expectedName)
			{
				// Append the arguments so that the sub shader processes them
				if (arguments.IsValid())
				{
					file << " ";
					file << arguments;
				}

				// Append this sub shader
				_Append(file);
			}
		}
	}

	// Whether the shader is actually already valid or not depends on whether it has any sub-shaders
	return mAdded.IsValid();
}

//============================================================================================================
// Only GLGraphics should be activating shaders
//============================================================================================================

GLShader* GLShader::Activate (const ITechnique* tech)
{
	// If the shader is marked as dirty, rebuild it (this also sets all flags)
	if (mIsDirty) _Rebuild();

	// If this is a surface shader, we might need to activate a different shader
	if (tech != 0 && GetFlag(IShader::Flag::Surface))
	{
		const String& shaderName = GetName();

		if (tech->GetFlag(ITechnique::Flag::Deferred))
		{
			if (mDeferred == 0)
			{
				// Remember this alternate version of the shader
				mDeferred = (GLShader*)mGraphics->GetShader(shaderName + " [Deferred]");

				// Copy over registered uniforms
				FOREACH(i, mUniforms)
				{
					GLShader::UniformEntry& ent = mUniforms[i];
					mDeferred->RegisterUniform(ent.mName, ent.mDelegate, ent.mSetOnDraw);
				}
			}
			mDeferred->_Activate();
			return mDeferred;
		}
		else if (tech->GetFlag(ITechnique::Flag::Shadowed))
		{
			if (mShadowed == 0)
			{
				mShadowed = (GLShader*)mGraphics->GetShader(shaderName + " [Shadowed]");

				FOREACH(i, mUniforms)
				{
					GLShader::UniformEntry& ent = mUniforms[i];
					mShadowed->RegisterUniform(ent.mName, ent.mDelegate, ent.mSetOnDraw);
				}
			}
			mShadowed->_Activate();
			return mShadowed;
		}
	}

	// No special technique -- activate this shader
	_Activate();
	return this;
}

//============================================================================================================
// Activate this program
//============================================================================================================

inline bool GLShader::_Activate()
{
	if (mIsDirty) _Rebuild();
	else if (g_activeProgram == mProgram) return (g_activeProgram != 0);

	// Activate this program
	glUseProgram(g_activeProgram = mProgram);
	CHECK_GL_ERROR;

	// If this is a valid program, update its uniforms
	if (g_activeProgram != 0)
	{
		Update(false);
		return true;
	}
	return false;
}

//============================================================================================================
// Rebuild the shader program
//============================================================================================================

bool GLShader::_Rebuild()
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

		if (_Link())
		{
			Update(false);
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Updates all uniforms
//============================================================================================================

uint GLShader::Update (bool aboutToDraw) const
{
	if (mDeferred != 0 && g_activeProgram == mDeferred->mProgram)
	{
		return mDeferred->Update(aboutToDraw);
	}
	else if (mShadowed != 0 && g_activeProgram == mShadowed->mProgram)
	{
		return mShadowed->Update(aboutToDraw);
	}
	else if (g_activeProgram != 0)
	{
		ASSERT(g_activeProgram == mProgram, "Wrong shader active?");

		uint count (0);
		Uniform uni;

		for (uint u = mUniforms.GetSize(); u > 0; )
		{
			UniformEntry& entry = mUniforms[--u];

			if (entry.mGLID != -1 && entry.mDelegate && entry.mSetOnDraw == aboutToDraw)
			{
				// Find the uniform if we have not yet tried to find it
				if (entry.mGLID == -2) entry.mGLID = ::GetUniformID(entry.mName);

				// If the uniform has been found, update it
				if (entry.mGLID != -1)
				{
					++count;
					uni.mType = Uniform::Type::Invalid;
					entry.mDelegate(entry.mName, uni);
					_UpdateUniform(entry.mGLID, uni);
					CHECK_GL_ERROR;
				}
			}
		}
		return count;
	}
	return 0;
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
	glBindAttribLocation(mProgram, IGraphics::Attribute::Vertex,		"R5_vertex");
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

		Array<String> lines;
		R5::CreateDebugLog(lines, log, "");

		// Print the debug log if there is something to print
		if (lines.IsValid())
		{
			FOREACH(i, lines)
			{
				if (lines[i].Contains("BindAttributeLocation")) continue;

				if (lines[i].Contains("Warning"))
				{
					WARNING(lines[i].GetBuffer());
				}
#ifdef _DEBUG
				else
				{
					System::Log("          - %s", lines[i].GetBuffer());
				}
#endif
			}
			System::FlushLog();
		}

		if (retVal == GL_TRUE)
		{
			// List the program's common supported features
			if (mFlags.Get(IShader::Flag::Skinned))		System::Log("          - Supports skinning");
		}
		else
		{
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
	::SetUniform1i(mProgram, "R5_shadowMap", 7);
	CHECK_GL_ERROR;
	return true;
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

void GLShader::_InsertUniform (const String& name, uint elements, const SetUniformDelegate& fnct, bool setOnDraw)
{
	UniformEntry& entry = mUniforms.Expand();
	entry.mName			= name;
	entry.mDelegate		= fnct;
	entry.mSetOnDraw	= setOnDraw;

	if (g_fillUniformList && elements > 0)
	{
		GLShader::UniformRecord& r = g_uniforms.Expand();
		r.name = name;
		r.elements = elements;
	}
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
	// Automatically delegate to hidden alternate versions of this shader if they are active instead
	if (mDeferred != 0 && g_activeProgram == mDeferred->mProgram) return mDeferred->SetUniform(name, uniform);
	if (mShadowed != 0 && g_activeProgram == mShadowed->mProgram) return mShadowed->SetUniform(name, uniform);

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

void GLShader::RegisterUniform (const String& name, const SetUniformDelegate& fnct, bool setOnDraw)
{
	bool found = false;

	for (uint i = 0; i < mUniforms.GetSize(); ++i)
	{
		UniformEntry& entry = mUniforms[i];

		if (entry.mName == name)
		{
			entry.mDelegate = fnct;
			entry.mSetOnDraw = setOnDraw;
			found = true;
			break;
		}
	}

	// This is a new value
	if (!found) _InsertUniform(name, 0, fnct, setOnDraw);

	// Ensure that associated shaders are also kept up to date
	if (mDeferred) mDeferred->RegisterUniform(name, fnct, setOnDraw);
	if (mShadowed) mShadowed->RegisterUniform(name, fnct, setOnDraw);
}