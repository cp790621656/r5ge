#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Find the OpenGL ID of the specified uniform entry
//============================================================================================================

inline int GetUniformID (uint program, const String& name)
{
	int glID = glGetUniformLocation(program, name.GetBuffer());
	CHECK_GL_ERROR;

#ifdef _DEBUG
	if (glID != -1) System::Log("          - Found uniform '%s' [%d]", name.GetBuffer(), glID);
#endif
	return glID;
}

//============================================================================================================
// Sets a uniform integer value in a shader
//============================================================================================================

inline void SetUniform1i (uint program, const char* name, int val)
{
	int loc = glGetUniformLocation(program, name);

	if (loc != -1)
	{
#ifdef _DEBUG
		System::Log("          - Found uniform '%s' [%u]", name, loc);
#endif
		glUniform1i(loc, val);
		CHECK_GL_ERROR;
	}
}

//============================================================================================================
// Helper function used in GLShaderUniforms::BindAttributes below
//============================================================================================================

inline void BindAttribute (const String& code, const uint& program, const uint& index, const char* name)
{
	if (code.Contains(name)) glBindAttribLocation(program, index, name);
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

void GLShaderUniforms::SetUniform_EyePos (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetCameraPosition();
}

//============================================================================================================
// Shader callback function for R5_pixelSize
//------------------------------------------------------------------------------------------------------------
// Can be used to figure out 0-1 range full-screen texture coordinates in the fragment shader:
// gl_FragCoord.xy * R5_pixelSize
//============================================================================================================

void GLShaderUniforms::SetUniform_PixelSize (const String& name, Uniform& uniform)
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

void GLShaderUniforms::SetUniform_ClipRange (const String& name, Uniform& uniform)
{
	const Vector3f& range = mGraphics->GetCameraRange();
	uniform = Quaternion(range.x, range.y, range.x * range.y, range.y - range.x);
}

//============================================================================================================
// Shader callback function for R5_fogRange
//============================================================================================================

void GLShaderUniforms::SetUniform_FogRange (const String& name, Uniform& uniform)
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

void GLShaderUniforms::SetUniform_FogColor (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetBackgroundColor();
}

//============================================================================================================
// Shader callback for R5_modelScale
//============================================================================================================

void GLShaderUniforms::SetUniform_MS (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix().GetScale();
}

//============================================================================================================
// Shader callback function for R5_modelMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_MM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelMatrix();
}

//============================================================================================================
// Shader callback for R5_viewMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_VM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetViewMatrix();
}

//============================================================================================================
// Shader callback for R5_projMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_PM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetProjectionMatrix();
}

//============================================================================================================
// Shader callback function for R5_modelViewMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_MVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelViewMatrix();
}

//============================================================================================================
// Shader callback for R5_modelViewProjMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_MVPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetModelViewProjMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseViewMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_IVM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseModelViewMatrix();
}

//============================================================================================================
// Shader callback function for R5_inverseViewRotationMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_IVRM (const String& name, Uniform& uniform)
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

void GLShaderUniforms::SetUniform_IPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseProjMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseMVPMatrix
//============================================================================================================

void GLShaderUniforms::SetUniform_IMVPM (const String& name, Uniform& uniform)
{
	uniform = mGraphics->GetInverseMVPMatrix();
}

//============================================================================================================
// Shader callback function for R5_lightAmbient
//============================================================================================================

void GLShaderUniforms::SetUniform_LightAmbient (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mAmbient;
	uniform.mType = Uniform::Type::Float3;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShaderUniforms::SetUniform_LightDiffuse (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mDiffuse;
	uniform.mType = Uniform::Type::Float3;
}

//============================================================================================================
// Shader callback function for R5_lightPosition
//============================================================================================================

void GLShaderUniforms::SetUniform_LightPosition (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mPos;
}

//============================================================================================================
// Shader callback function for R5_lightDirection
//============================================================================================================

void GLShaderUniforms::SetUniform_LightDirection (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = -light.mDir;
}

//============================================================================================================
// Shader callback function for R5_lightDiffuse
//============================================================================================================

void GLShaderUniforms::SetUniform_LightParams (const String& name, Uniform& uniform)
{
	const ILight& light = mGraphics->GetActiveLight(0);
	uniform = light.mParams;
}

//============================================================================================================
// Shader callback function for R5_materialColor
//============================================================================================================

void GLShaderUniforms::SetUniform_MatColor (const String& name, Uniform& uniform)
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

void GLShaderUniforms::SetUniform_MatParams0 (const String& name, Uniform& uniform)
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

void GLShaderUniforms::SetUniform_MatParams1 (const String& name, Uniform& uniform)
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
// Add a new registered uniform value without checking to see if it already exists
//============================================================================================================

inline void GLShaderUniforms::Insert (const String& name, const SetUniformDelegate& fnct, uint group)
{
	Entry& entry	= mList.Expand();
	entry.mName		= name;
	entry.mDelegate	= fnct;
	entry.mGroup	= group;
	entry.mGLID		= -2;
}

//============================================================================================================
// Register all built-in uniforms
//============================================================================================================

void GLShaderUniforms::RegisterBuiltInUniforms()
{
	// These uniforms will be set on IShader::_Activate()
	Insert( "R5_time",				&SetUniform_Time, false );
	Insert( "R5_eyePosition",		bind(&GLShaderUniforms::SetUniform_EyePos,			this), Uniform::Group::SetWhenActivated );
	Insert( "R5_pixelSize",			bind(&GLShaderUniforms::SetUniform_PixelSize,		this), Uniform::Group::SetWhenActivated );
	Insert( "R5_clipRange",			bind(&GLShaderUniforms::SetUniform_ClipRange,		this), Uniform::Group::SetWhenActivated );
	Insert( "R5_fogRange",			bind(&GLShaderUniforms::SetUniform_FogRange,		this), Uniform::Group::SetWhenActivated );
	Insert( "R5_fogColor",			bind(&GLShaderUniforms::SetUniform_FogColor,		this), Uniform::Group::SetWhenActivated );

	// These uniforms will be set on IShader::Update(), which happens just before the drawing operations
	Insert( "R5_materialColor",		bind(&GLShaderUniforms::SetUniform_MatColor,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_materialParams0",	bind(&GLShaderUniforms::SetUniform_MatParams0,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_materialParams1",	bind(&GLShaderUniforms::SetUniform_MatParams1,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_lightAmbient",		bind(&GLShaderUniforms::SetUniform_LightAmbient,	this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_lightDiffuse",		bind(&GLShaderUniforms::SetUniform_LightDiffuse,	this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_lightPosition",		bind(&GLShaderUniforms::SetUniform_LightPosition,	this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_lightDirection",	bind(&GLShaderUniforms::SetUniform_LightDirection,	this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_lightParams",		bind(&GLShaderUniforms::SetUniform_LightParams,		this), Uniform::Group::SetWhenDrawing );

	// All matrices must be updated prior to each draw call as well
	Insert( "R5_modelScale",				bind(&GLShaderUniforms::SetUniform_MS,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_modelMatrix",				bind(&GLShaderUniforms::SetUniform_MM,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_viewMatrix",				bind(&GLShaderUniforms::SetUniform_VM,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_projMatrix",				bind(&GLShaderUniforms::SetUniform_PM,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_modelViewMatrix",			bind(&GLShaderUniforms::SetUniform_MVM,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_modelViewProjMatrix",		bind(&GLShaderUniforms::SetUniform_MVPM,	this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_inverseViewMatrix",			bind(&GLShaderUniforms::SetUniform_IVM,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_inverseProjMatrix",			bind(&GLShaderUniforms::SetUniform_IPM,		this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_inverseMVPMatrix",			bind(&GLShaderUniforms::SetUniform_IMVPM,	this), Uniform::Group::SetWhenDrawing );
	Insert( "R5_inverseViewRotationMatrix",	bind(&GLShaderUniforms::SetUniform_IVRM,	this), Uniform::Group::SetWhenDrawing );
}

//============================================================================================================
// Bind all one-time attributes prior to linking the program
//============================================================================================================

void GLShaderUniforms::BindAttributes (uint program, const String& code)
{
	::BindAttribute(code, program, IGraphics::Attribute::Vertex,			"R5_position");
	::BindAttribute(code, program, IGraphics::Attribute::Tangent,			"R5_tangent");
	::BindAttribute(code, program, IGraphics::Attribute::Normal,			"R5_normal");
	::BindAttribute(code, program, IGraphics::Attribute::Color,				"R5_color");
	::BindAttribute(code, program, IGraphics::Attribute::SecondaryColor,	"R5_secondaryColor");
	::BindAttribute(code, program, IGraphics::Attribute::FogCoord,			"R5_fogCoord");
	::BindAttribute(code, program, IGraphics::Attribute::BoneWeight,		"R5_boneWeight");
	::BindAttribute(code, program, IGraphics::Attribute::BoneIndex,			"R5_boneIndex");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord0,			"R5_texCoord0");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord1,			"R5_texCoord1");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord2,			"R5_texCoord2");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord3,			"R5_texCoord3");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord4,			"R5_texCoord4");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord5,			"R5_texCoord5");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord6,			"R5_texCoord6");
	::BindAttribute(code, program, IGraphics::Attribute::TexCoord7,			"R5_texCoord7");
}

//============================================================================================================
// Set the constant values of texture units in the shader
//============================================================================================================

void GLShaderUniforms::BindTextureUnits (uint program)
{
	::SetUniform1i(program, "R5_texture0",  0);
	::SetUniform1i(program, "R5_texture1",  1);
	::SetUniform1i(program, "R5_texture2",  2);
	::SetUniform1i(program, "R5_texture3",  3);
	::SetUniform1i(program, "R5_texture4",  4);
	::SetUniform1i(program, "R5_texture5",  5);
	::SetUniform1i(program, "R5_texture6",  6);
	::SetUniform1i(program, "R5_shadowMap", 7);
}

//============================================================================================================
// Set the value of the specified uniform
//============================================================================================================

bool GLShaderUniforms::Set (uint program, const String& name, const Uniform& uniform)
{
	FOREACH(i, mList)
	{
		const Entry& entry = mList[i];

		if (entry.mName == name)
		{
			// The uniform has not yet been located
			if (entry.mGLID == -2) entry.mGLID = ::GetUniformID(program, name);

			// The uniform does not exist
			if (entry.mGLID == -1) return false;

			// The uniform exists and can be updated
			return Update(entry.mGLID, uniform);
		}
	}

	// This must be a new entry -- try to add it
	Entry& entry = mList.Expand();
	entry.mName = name;
	entry.mGLID = ::GetUniformID(program, name);
	entry.mGroup = Uniform::Group::SetManually;
	return (entry.mGLID != -1) ? Update(entry.mGLID, uniform) : false;
}

//============================================================================================================
// Update all the registered uniforms belonging to the specified group
//============================================================================================================

uint GLShaderUniforms::Update (uint program, uint group) const
{
	uint count (0);
	Uniform uni;

	FOREACH(i, mList)
	{
		const Entry& entry = mList[i];

		if (entry.mGroup == group)
		{
			if (entry.mGLID == -2)
			{
				entry.mGLID = ::GetUniformID(program, entry.mName);
			}

			if (entry.mGLID != -1)
			{
				++count;
				uni.mType = Uniform::Type::Invalid;
				entry.mDelegate(entry.mName, uni);
				Update(entry.mGLID, uni);
				CHECK_GL_ERROR;
			}
		}
	}
	return count;
}

//============================================================================================================
// Update the specified uniform
//============================================================================================================

bool GLShaderUniforms::Update (uint index, const Uniform& uni) const
{
	if (index >= 0)
	{
		switch (uni.mType)
		{
		case Uniform::Type::Float1:
			glUniform1f(index, uni.mVal[0]);
			break;
		case Uniform::Type::Float2:
			glUniform2f(index, uni.mVal[0], uni.mVal[1]);
			break;
		case Uniform::Type::Float3:
			glUniform3f(index, uni.mVal[0], uni.mVal[1], uni.mVal[2]);
			break;
		case Uniform::Type::Float4:
			glUniform4f(index, uni.mVal[0], uni.mVal[1], uni.mVal[2], uni.mVal[3]);
			break;
		case Uniform::Type::Float9:
			glUniformMatrix3fv(index, 1, 0, uni.mVal);
			break;
		case Uniform::Type::Float16:
			glUniformMatrix4fv(index, 1, 0, uni.mVal);
			break;
		case Uniform::Type::ArrayFloat1:
			glUniform1fv(index, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat2:
			glUniform2fv(index, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat3:
			glUniform3fv(index, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat4:
			glUniform4fv(index, uni.mElements, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat9:
			glUniformMatrix3fv(index, uni.mElements, 0, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayFloat16:
			glUniformMatrix4fv(index, uni.mElements, 0, (float*)uni.mPtr);
			break;
		case Uniform::Type::ArrayInt:
			glUniform1iv(index, uni.mElements, (int*)uni.mPtr);
			break;
		}
		CHECK_GL_ERROR;
		return true;
	}
	return false;
}

//============================================================================================================
// Registers a uniform variable that's updated once per frame
//============================================================================================================

void GLShaderUniforms::Register (const String& name, const SetUniformDelegate& fnct, uint group)
{
	FOREACH(i, mList)
	{
		Entry& entry = mList[i];

		if (entry.mName == name)
		{
			entry.mDelegate = fnct;
			entry.mGroup = group;
			return;
		}
	}

	// Add a new entry
	Insert(name, fnct, group);
}