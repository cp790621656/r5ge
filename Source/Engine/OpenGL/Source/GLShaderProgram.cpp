#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

// Gets the source code for the specified internal shader
extern uint GLGetInternalShaderCode (const String& name, String& code);

// Preprocess the specified shader code, 
extern uint GLPreprocessShader (String& source, const Flags& desired, Flags& final);

//============================================================================================================
// Keep track of the currently active program
//============================================================================================================

uint g_activeProgram = 0;

//============================================================================================================
// For the sake of not duplicating data, all R5 built-ins are stored in a library-accessible list.
//============================================================================================================

bool g_fillUniformList = true;
Array<GLShaderProgram::UniformRecord> g_uniforms;

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

void GLShaderProgram::SetUniform_EyePos (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetCameraPosition();
}

//============================================================================================================
// Shader callback function for R5_pixelSize
//------------------------------------------------------------------------------------------------------------
// Can be used to figure out 0-1 range full-screen texture coordinates in the fragment shader:
// gl_FragCoord.xy * R5_pixelSize
//============================================================================================================

void GLShaderProgram::SetUniform_PixelSize (const String& name, Uniform& uniform)
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

void GLShaderProgram::SetUniform_ClipRange (const String& name, Uniform& uniform)
{
	const Vector3f& range = mGraphics->GetCameraRange();
	uniform = Quaternion(range.x, range.y, range.x * range.y, range.y - range.x);
}

//============================================================================================================
// Shader callback function for R5_fogRange
//============================================================================================================

void GLShaderProgram::SetUniform_FogRange (const String& name, Uniform& uniform)
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

void GLShaderProgram::SetUniform_FogColor (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetBackgroundColor();
}

//============================================================================================================
// Shader callback for R5_modelScale
//============================================================================================================

void GLShaderProgram::SetUniform_MS (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix().GetScale();
}

//============================================================================================================
// Shader callback function for R5_modelMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_MM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix();
}

//============================================================================================================
// Shader callback for R5_viewMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_VM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetViewMatrix();
}

//============================================================================================================
// Shader callback for R5_projMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_PM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetProjectionMatrix();
}

//============================================================================================================
// Shader callback function for R5_modelViewMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_MVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelViewMatrix();
}

//============================================================================================================
// Shader callback for R5_modelViewProjMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_MVPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelViewProjMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseViewMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_IVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseModelViewMatrix();
}

//============================================================================================================
// Shader callback function for R5_inverseViewRotationMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_IVRM (const String& name, Uniform& uniform)
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

void GLShaderProgram::SetUniform_IPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseProjMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseMVPMatrix
//============================================================================================================

void GLShaderProgram::SetUniform_IMVPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseMVPMatrix();
}

//============================================================================================================
// Shader callback function for R5_lightAmbient
//============================================================================================================

void GLShaderProgram::SetUniform_LightAmbient (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mAmbient;
	uniform.mType = Uniform::Type::Float3;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShaderProgram::SetUniform_LightDiffuse (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mDiffuse;
	uniform.mType = Uniform::Type::Float3;
}

//============================================================================================================
// Shader callback function for R5_lightPosition
//============================================================================================================

void GLShaderProgram::SetUniform_LightPosition (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mPos;
}

//============================================================================================================
// Shader callback function for R5_lightDirection
//============================================================================================================

void GLShaderProgram::SetUniform_LightDirection (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = -light.mDir;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShaderProgram::SetUniform_LightParams (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mParams;
}

//============================================================================================================
// Shader callback function for R5_materialColor
//============================================================================================================

void GLShaderProgram::SetUniform_MatColor (const String& name, Uniform& uniform)
{
	const IMaterial* mat = mGraphics->GetActiveMaterial();

	if (mat == 0)
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

void GLShaderProgram::SetUniform_MatParams0 (const String& name, Uniform& uniform)
{
	const IMaterial* mat = mGraphics->GetActiveMaterial();

	if (mat == 0)
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

void GLShaderProgram::SetUniform_MatParams1 (const String& name, Uniform& uniform)
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

void GLShaderProgram::_Init (GLGraphics* graphics)
{
	mGraphics = graphics;

	// These uniforms will be set on IShader::_Activate()
	_InsertUniform( "R5_time",				3,  &SetUniform_Time, false );
	_InsertUniform( "R5_eyePosition",		3,  bind(&GLShaderProgram::SetUniform_EyePos,			this), Uniform::Group::SetWhenActivated );
	_InsertUniform( "R5_pixelSize",			2,  bind(&GLShaderProgram::SetUniform_PixelSize,		this), Uniform::Group::SetWhenActivated );
	_InsertUniform( "R5_clipRange",			4,  bind(&GLShaderProgram::SetUniform_ClipRange,		this), Uniform::Group::SetWhenActivated );
	_InsertUniform( "R5_fogRange",			2,  bind(&GLShaderProgram::SetUniform_FogRange,			this), Uniform::Group::SetWhenActivated );
	_InsertUniform( "R5_fogColor",			4,  bind(&GLShaderProgram::SetUniform_FogColor,			this), Uniform::Group::SetWhenActivated );

	// These uniforms will be set on IShader::Update(), which happens just before the drawing operations
	_InsertUniform( "R5_materialColor",		4,  bind(&GLShaderProgram::SetUniform_MatColor,			this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_materialParams0",	4,  bind(&GLShaderProgram::SetUniform_MatParams0,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_materialParams1",	2,  bind(&GLShaderProgram::SetUniform_MatParams1,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_lightAmbient",		3,  bind(&GLShaderProgram::SetUniform_LightAmbient,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_lightDiffuse",		3,  bind(&GLShaderProgram::SetUniform_LightDiffuse,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_lightPosition",		3,  bind(&GLShaderProgram::SetUniform_LightPosition,	this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_lightDirection",	3,  bind(&GLShaderProgram::SetUniform_LightDirection,	this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_lightParams",		3,  bind(&GLShaderProgram::SetUniform_LightParams,		this), Uniform::Group::SetWhenDrawing );

	// All matrices must be updated prior to each draw call as well
	_InsertUniform( "R5_modelScale",				3,  bind(&GLShaderProgram::SetUniform_MS,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_modelMatrix",				16, bind(&GLShaderProgram::SetUniform_MM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_viewMatrix",				16, bind(&GLShaderProgram::SetUniform_VM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_projMatrix",				16, bind(&GLShaderProgram::SetUniform_PM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_modelViewMatrix",			16, bind(&GLShaderProgram::SetUniform_MVM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_modelViewProjMatrix",		16, bind(&GLShaderProgram::SetUniform_MVPM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_inverseViewMatrix",			16, bind(&GLShaderProgram::SetUniform_IVM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_inverseProjMatrix",			16, bind(&GLShaderProgram::SetUniform_IPM,		this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_inverseMVPMatrix",			16, bind(&GLShaderProgram::SetUniform_IMVPM,	this), Uniform::Group::SetWhenDrawing );
	_InsertUniform( "R5_inverseViewRotationMatrix",	9,  bind(&GLShaderProgram::SetUniform_IVRM,		this), Uniform::Group::SetWhenDrawing );

	// The list of R5 uniforms should now be complete
	g_fillUniformList = false;
}

//============================================================================================================
// Initialize the shader
//============================================================================================================

void GLShaderProgram::_Init (GLGraphics* graphics, GLShaderProgram* parent, const String& name, const Flags& desired)
{
	mParent		= parent;
	mName		= name;
	mDesired	= desired;

	_Init(graphics);
}

//============================================================================================================
// Only GLGraphics should be activating shaders
//============================================================================================================

GLShaderProgram* GLShaderProgram::_Activate (const ITechnique* tech)
{
	// Check for the shader's source code if needed
	if (mCheckForSource) _CheckForSource();

	// If the shader is marked as dirty, rebuild it (this also sets all flags)
	if (mNeedsLinking) _ReLink();

	// If this is a surface shader, we might need to activate a different shader
	if (tech != 0 && GetFlag(IShader::Flag::Surface))
	{
		Flags desired;

		if (tech->GetFlag(ITechnique::Flag::Deferred))
		{
			// Deferred rendering variation
			desired.Set(IShader::Flag::Deferred, true);
		}
		else
		{
			// If a light is active, automatically request the appropriate variation
			const ILight& light = mGraphics->GetActiveLight(0);

			if (light.mType == ILight::Type::Directional)
			{
				desired.Set(IShader::Flag::DirLight, true);

				if (tech->GetFlag(ITechnique::Flag::Shadowed))
				{
					desired.Set(IShader::Flag::Shadowed, true);
				}
			}
			else if (light.mType == ILight::Type::Point)
			{
				desired.Set(IShader::Flag::PointLight, true);
			}
			//else if (light.mType == ILight::Type::Spot)
			//{
			//	desired.Set(IShader::Flag::SpotLight, true);
			//}
		}

		// Call the appropriate variation
		FOREACH(i, mVariations)
		{
			Variation& v = mVariations[i];

			if (v.flags == desired)
			{
				v.shader->_Activate();
				return v.shader;
			}
		}

		// No such variation exists -- add a new entry
		{
			Variation& v = mVariations.Expand();
			v.flags = desired;

			// Find or create this special shader
			{
				FOREACH(i, mSpecial)
				{
					GLShaderProgram* shd = mSpecial[i];

					if (shd->mDesired == desired)
					{
						v.shader = shd;
						break;
					}
				}

				if (v.shader == 0)
				{
					v.shader = new GLShaderProgram();
					v.shader->_Init(mGraphics, this, mName, desired);
					mSpecial.Expand() = v.shader;

					// Copy over registered uniforms
					FOREACH(i, mUniforms)
					{
						GLShaderProgram::UniformEntry& ent = mUniforms[i];
						v.shader->RegisterUniform(ent.mName, ent.mDelegate, ent.mGroup);
					}
				}
			}

			v.shader->_Activate();
			return v.shader;
		}
	}

	// No special technique -- activate this shader
	_Activate();
	return this;
}

//============================================================================================================
// INTERNAL: Sets the shader's source code by either using the parent's, or finding it
//============================================================================================================

void GLShaderProgram::_CheckForSource()
{
	mCheckForSource = 0;

	if (mParent == 0)
	{
		// Shaders that begin with [R5] are built-in shaders
		if (mName.BeginsWith("[R5]"))
		{
			// Shaders with names that begin with [R5] are internal shaders
			String code;
			::GLGetInternalShaderCode(mName, code);
			if (code.IsValid()) SetComponentCode(code);
		}
		else
		{
			// Clean up the filename, removing parameters specified in square brackets
			String path (System::GetPathFromFilename(mName));
			path << System::GetFilenameFromPath(mName, true);

			// Collect the files matching the specified path
			Array<String> files;
			System::GetFiles(path, files, true);
			System::GetFiles("Shaders/" + path, files, true);

			// This is the expected filename
			String code, expectedName (System::GetFilenameFromPath(mName, false));

			// Run through all located files and use shaders with filenames matching the expected value
			FOREACH(i, files)
			{
				const String& file = files[i];
				String filename (System::GetFilenameFromPath(file, false));

				if (filename == expectedName && code.Load(file))
				{
					SetComponentCode(code);
				}
			}
		}

		if (mAdded.IsEmpty())
		{
			WARNING((mName + " is missing it source code").GetBuffer());
		}
	}
	else
	{
		ASSERT(mParent->mCode.IsValid(), "Missing source code in the parent?");
		mAdded.Release();

		FOREACH(i, mParent->mCode)
		{
			CodeEntry& ent = mParent->mCode[i];
			SetComponentCode(ent.mCode);
		}
	}
}

//============================================================================================================
// Activate this program
//============================================================================================================

inline bool GLShaderProgram::_Activate()
{
	if (mCheckForSource) _CheckForSource();
	if (mNeedsLinking) _ReLink();
	else if (g_activeProgram == mProgram) return (g_activeProgram != 0);

	// Activate this program
	glUseProgram(g_activeProgram = mProgram);
	CHECK_GL_ERROR;
	return (g_activeProgram != 0);
}

//============================================================================================================
// Rebuild the shader program
//============================================================================================================

inline bool GLShaderProgram::_ReLink()
{
	mNeedsLinking = 0;
	_Detach();
	return (mAdded.IsValid()) ? _Link() : false;
}

//============================================================================================================
// Updates all uniforms
//============================================================================================================

uint GLShaderProgram::_Update (uint group) const
{
	ASSERT(mProgram != 0, "Updating a shader that hasn't been created");

	if (g_activeProgram == mProgram)
	{
		uint count (0);
		Uniform uni;

		for (uint u = mUniforms.GetSize(); u > 0; )
		{
			UniformEntry& entry = mUniforms[--u];

			if (entry.mGLID != -1 && entry.mDelegate && entry.mGroup == group)
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
	else
	{
		FOREACH(i, mVariations)
		{
			Variation& v = mVariations[i];

			if (g_activeProgram == v.shader->mProgram)
			{
				return v.shader->_Update(group);
			}
		}
	}
	return 0;
}

//============================================================================================================
// Deactivates the current shader
//============================================================================================================

void GLShaderProgram::_Deactivate() const
{
	if (g_activeProgram != 0) glUseProgram(g_activeProgram = 0);
}

//============================================================================================================
// INTERNAL: Detaches the attached shaders
//============================================================================================================

void GLShaderProgram::_Detach()
{
	for (uint i = mAttached.GetSize(); i > 0; )
	{
		GLShaderComponent* sub = mAttached[--i];

		if (sub->mGLID != INVALID_VAL)
		{
			glDetachShader(mProgram, sub->mGLID);
			CHECK_GL_ERROR;
		}
	}
	mAttached.Clear();
}

//============================================================================================================
// INTERNAL: Releases all resources used by the shader
//============================================================================================================

void GLShaderProgram::_Release()
{
	mNeedsLinking = 0;
	mAdded.Release();

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

bool GLShaderProgram::_Validate (PointerArray<GLShaderComponent>& list)
{
	for (uint i = list.GetSize(); i > 0; )
	{
		GLShaderComponent* sub = list[--i];

		if (!sub->IsValid())
		{
			ASSERT(false, "Attempting to use a missing shader!");
			_Deactivate();
			return false;
		}
	}
	return true;
}

//============================================================================================================
// INTERNAL: Attach all GLSL shaders to this shader program
//============================================================================================================

void GLShaderProgram::_Attach (PointerArray<GLShaderComponent>& list)
{
	for (uint i = list.GetSize(); i > 0; )
	{
		GLShaderComponent* sub = list[--i];
		glAttachShader(mProgram, sub->mGLID);
		mFinal.Include(sub->mFlags);
		mAttached.Expand() = sub;
	}
}

//============================================================================================================
// Link all shaders
//============================================================================================================

bool GLShaderProgram::_Link()
{
	mNeedsLinking = 0;

	// Ensure that the added shaders pass validation
	if (!_Validate(mAdded)) return false;

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
			GLShaderComponent* sub = mAttached[i];
			const char* type = (sub->mType == IShader::Type::Vertex) ? "Vertex" : "Fragment";
			if (sub->mType == IShader::Type::Geometry) type = "Geometry";
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
			if (mFinal.Get(IShader::Flag::Surface))		System::Log("          - Surface shader");
			if (mFinal.Get(IShader::Flag::Deferred))	System::Log("          - Deferred approach");
			if (mFinal.Get(IShader::Flag::Lit))			System::Log("          - Supports lighting");
			if (mFinal.Get(IShader::Flag::Shadowed))	System::Log("          - Supports shadows");
			if (mFinal.Get(IShader::Flag::Skinned))		System::Log("          - Supports skinning");
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

bool GLShaderProgram::_UpdateUniform (uint glID, const Uniform& uni) const
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

void GLShaderProgram::_InsertUniform (const String& name, uint elements, const SetUniformDelegate& fnct, uint group)
{
	UniformEntry& entry = mUniforms.Expand();
	entry.mName			= name;
	entry.mDelegate		= fnct;
	entry.mGroup		= group;

	if (g_fillUniformList && elements > 0)
	{
		GLShaderProgram::UniformRecord& r = g_uniforms.Expand();
		r.name = name;
		r.elements = elements;
	}
}

//============================================================================================================
// INTERNAL: Gets or creates a shader code entry of specified type
//============================================================================================================

GLShaderProgram::CodeEntry& GLShaderProgram::_GetCodeEntry (uint type)
{
	ASSERT(type != IShader::Type::Unknown, "Invalid shader type specified");

	// Surface shaders replace all shader types
	if (type == IShader::Type::Surface) mCode.Clear();

	// Locate the previous entry for a shader component of this type
	FOREACH(i, mCode)
	{
		CodeEntry& ent = mCode[i];
		if (ent.mType == type) return ent;
	}

	// Create a new shader code entry
	{
		CodeEntry& ent = mCode.Expand();
		ent.mType = type;
		return ent;
	}
}

//============================================================================================================
// INTERNAL: Gets or creates a shader component of specified type
//============================================================================================================

GLShaderComponent* GLShaderProgram::_GetComponent (uint type)
{
	ASSERT(type != IShader::Type::Unknown, "Invalid shader type specified");

	// Find an existing component entry for this shader
	FOREACH(i, mAdded)
	{
		GLShaderComponent* comp = mAdded[i];
		if (comp != 0 && comp->mType == type) return comp;
	}

	// Create a new shader component entry
	{
		GLShaderComponent* comp = new GLShaderComponent(this, mName);
		mAdded.Expand() = comp;
		return comp;
	}
}

//============================================================================================================
// Sets the shader source code, hinting that the code if of specified type
//============================================================================================================

uint GLShaderProgram::SetComponentCode (const String& code)
{
	if (code.IsEmpty()) return Type::Unknown;

	// First, pre-process the shader, converting provided code into GLSL
	// TODO: What if 'code' is a surface shader?
	Flags final;
	String modified (code);
	uint type = ::GLPreprocessShader(modified, mDesired, final);

	// If this is the original shader, save the source code
	if (mParent == 0) _GetCodeEntry(type).mCode = code;

	// Update the shader component code
	GLShaderComponent* comp = _GetComponent(type);
	comp->SetCode(modified, type, final);
	mNeedsLinking = 1;

	// All special shaders must also be invalidated
	FOREACH(i, mSpecial) mSpecial[i]->mCheckForSource = 1;
	return type;
}

//============================================================================================================
// Retrieves the source code of the specified shader component
//============================================================================================================

const String& GLShaderProgram::GetComponentCode (uint type) const
{
	static String empty;

	FOREACH(i, mCode)
	{
		const CodeEntry& ent = mCode[i];
		if (ent.mType == type) return ent.mCode;
	}
	return empty;
}

//============================================================================================================
// Force-updates the value of the specified uniform
//============================================================================================================

bool GLShaderProgram::SetUniform (const String& name, const Uniform& uniform) const
{
	ASSERT(g_activeProgram != 0, "No shader is active");

	if (g_activeProgram == mProgram)
	{
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
	}
	else
	{
		FOREACH(i, mVariations)
		{
			Variation& v = mVariations[i];

			if (g_activeProgram == v.shader->mProgram)
			{
				return v.shader->SetUniform(name, uniform);
			}
		}
	}
	return false;
}

//============================================================================================================
// Registers a uniform variable that's updated once per frame
//============================================================================================================

void GLShaderProgram::RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint group)
{
	bool found = false;

	for (uint i = 0; i < mUniforms.GetSize(); ++i)
	{
		UniformEntry& entry = mUniforms[i];

		if (entry.mName == name)
		{
			entry.mDelegate = fnct;
			entry.mGroup = group;
			found = true;
			break;
		}
	}

	// This is a new value
	if (!found) _InsertUniform(name, 0, fnct, group);

	// Ensure that all variations are also kept up to date
	FOREACH(i, mVariations)
	{
		Variation& v = mVariations[i];
		v.shader->RegisterUniform(name, fnct, group);
	}
}
