#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

// External functions
extern uint GLGetInternalShaderCode (String& code, const String& name);
extern Flags GLPreprocessShader (String& code, const Flags& desired);

GLShaderProgram* g_lastProgram = 0;

//============================================================================================================
// Activate this shader for the specified technique
//============================================================================================================

bool GLSurfaceShader::Activate (const ITechnique* tech)
{
	if (mCheckForSource)
	{
		mCheckForSource = false;
		CheckForSource();
	}

	// If the shader is marked as dirty, release all of its current components
	if (mIsDirty)
	{
		mIsDirty = false;
		mPrograms.Release();
		mComponents.Release();
	}

	if (!mHasErrors)
	{
		Flags desired;

		// Compile the list of desired flags for this shader
		if (tech != 0 && mBasicFlags.Get(IShader::Flag::Surface))
		{
			if (tech->GetFlag(ITechnique::Flag::Deferred))
			{
				// Deferred rendering variation
				desired.Set(IShader::Flag::Deferred, true);
			}
			else
			{
				if (!tech->GetColorWrite())
				{
					// Color write is turned off -- render only to depth
					desired.Set(IShader::Flag::DepthOnly, true);
				}
				else
				{
					// Whether we want fog
					if (tech->GetFog()) desired.Set(IShader::Flag::Fog, true);

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
			}
		}

		// Get the appropriate shader program
		GLShaderProgram* pro = GetProgram(desired);

		// Activate the program
		if (pro != 0)
		{
			if (pro->Activate())
			{
				g_lastProgram = pro;
				return true;
			}
			else
			{
				g_lastProgram = 0;
				return false;
			}
		}
	}
	// If this point was reached then the shader is invalid
	Deactivate();
	return false;
}

//============================================================================================================
// Deactivate the currently active shader
//============================================================================================================

inline void GLSurfaceShader::Deactivate() const
{
	if (g_lastProgram != 0)
	{
		g_lastProgram->Deactivate();
		g_lastProgram = 0;
	}
}

//============================================================================================================
// Try to find the source code for this shader
//============================================================================================================

void GLSurfaceShader::CheckForSource()
{
	// Shaders that begin with [R5] are built-in shaders
	if (mName.BeginsWith("[R5]"))
	{
		// Shaders with names that begin with [R5] are internal shaders
		String code;
		::GLGetInternalShaderCode(code, mName);
		if (code.IsValid()) SetCode(code);
	}
	else
	{
		// Clean up the filename, removing parameters specified in square brackets
		String path (System::GetPathFromFilename(mName));
		path << System::GetFilenameFromPath(mName, true);

		// This is the expected filename
		String code, expectedName (System::GetBestMatch(path));
		if (expectedName.IsEmpty()) expectedName = System::GetBestMatch("Shaders/" + path);

		// Load the file
		if (expectedName.IsValid() && code.Load(expectedName)) SetCode(code);
	}
}

//============================================================================================================
// Save a variation of the shader into the specified string
//============================================================================================================

Flags GLSurfaceShader::GetVariation (String& out, const Flags& flags) const
{
	out.Clear();

	if (flags == 0)
	{
		mCode.SerializeTo(out);
	}
	else
	{
		Array<String> defines;

		if (flags.Get(IShader::Flag::Vertex))		defines.Expand() = "Vertex";
		if (flags.Get(IShader::Flag::Fragment))		defines.Expand() = "Fragment";
		if (flags.Get(IShader::Flag::Geometry))		defines.Expand() = "Geometry";
		if (flags.Get(IShader::Flag::LegacyFormat)) defines.Expand() = "LegacyFormat";
		if (flags.Get(IShader::Flag::Skinned))		defines.Expand() = "Skinned";
		if (flags.Get(IShader::Flag::Billboard))	defines.Expand() = "Billboard";
		if (flags.Get(IShader::Flag::Shadowed))		defines.Expand() = "Shadowed";
		if (flags.Get(IShader::Flag::Fog))			defines.Expand() = "Fog";
		if (flags.Get(IShader::Flag::Deferred))		defines.Expand() = "Deferred";
		if (flags.Get(IShader::Flag::DirLight))		defines.Expand() = "DirLight";
		if (flags.Get(IShader::Flag::PointLight))	defines.Expand() = "PointLight";
		if (flags.Get(IShader::Flag::SpotLight))	defines.Expand() = "SpotLight";
		if (flags.Get(IShader::Flag::Lit))			defines.Expand() = "Lit";
		if (flags.Get(IShader::Flag::Surface))		defines.Expand() = "Surface";
		if (flags.Get(IShader::Flag::DepthOnly))	defines.Expand() = "DepthOnly";

		mCode.SerializeTo(out, defines);
	}
	return ::GLPreprocessShader(out, flags);
}

//============================================================================================================
// Gets or creates a shader program given the specified set of flags
//============================================================================================================

GLShaderProgram* GLSurfaceShader::GetProgram (const Flags& flags)
{
	// Check to see if this program already exists
	FOREACH(i, mPrograms)
	{
		GLShaderProgram* p = mPrograms[i];

		FOREACH(b, p->mMatches)
		{
			const Flags& match = p->mMatches[b];
			if (flags == match) return p;
		}
	}

	// Get the shader components matching the specified flags
	GLShaderComponent* vert = (mBasicFlags.Get(IShader::Flag::Vertex)) ?
		GetComponent(flags | IShader::Flag::Vertex) : 0;

	GLShaderComponent* frag = (mBasicFlags.Get(IShader::Flag::Fragment)) ?
		GetComponent(flags | IShader::Flag::Fragment) : 0;

	GLShaderComponent* geom = (mBasicFlags.Get(IShader::Flag::Geometry)) ?
		GetComponent(flags | IShader::Flag::Geometry) : 0;

	if (vert == 0 && frag == 0 && geom == 0)
	{
		mHasErrors = true;
		ASSERT(false, "Unable to find suitable source code for the shader");
		return 0;
	}

	// Check to see if there is already a program using these components
	FOREACH(i, mPrograms)
	{
		GLShaderProgram* p = mPrograms[i];

		if (p->mVert == vert &&
			p->mFrag == frag &&
			p->mGeom == geom)
		{
			// This program will now match the specified flags as well
			p->mMatches.Expand() = flags;
			return p;
		}
	}

	// This is a new program -- add it
	GLShaderProgram* pro = new GLShaderProgram(mGraphics);
	pro->mVert = vert;
	pro->mFrag = frag;
	pro->mGeom = geom;
	pro->mMatches.Expand() = flags;

	// Copy over the uniforms
	FOREACH(i, mUniforms)
	{
		RegisteredUniform& uni = mUniforms[i];
		pro->mUniforms.Register(uni.mName, uni.mDelegate, uni.mGroup);
	}

	// Save the program
	mPrograms.Expand() = pro;
	return pro;
}

//============================================================================================================
// Gets or creates a shader component given the specified set of flags
//============================================================================================================

GLShaderComponent* GLSurfaceShader::GetComponent (const Flags& flags)
{
	String code;
	Flags actual = GetVariation(code, flags);

	FOREACH(i, mComponents)
	{
		GLShaderComponent* c = mComponents[i];
		if (c->GetCode() == code) return c;
	}

	GLShaderComponent* comp = new GLShaderComponent(mName);
	comp->SetCode(code, actual);
	mComponents.Expand() = comp;
	return comp;
}

//============================================================================================================
// Update the specified uniform group
//============================================================================================================

void GLSurfaceShader::UpdateUniforms (uint group) const
{
	ASSERT(g_lastProgram != 0, "Missing program?");
	if (g_lastProgram != 0) g_lastProgram->UpdateUniforms(group);
}

//============================================================================================================
// Release all shaders, then all programs
//============================================================================================================

GLSurfaceShader::~GLSurfaceShader()
{
	mPrograms.Release();
	mComponents.Release();
}

//============================================================================================================
// Should match against the active shader's flags
//============================================================================================================

bool GLSurfaceShader::GetFlag (uint val) const
{
	return (g_lastProgram != 0) ? g_lastProgram->mFlags.Get(val) : false;
}

//============================================================================================================
// Sets the shader's source code
//============================================================================================================

void GLSurfaceShader::SetCode (const String& code)
{
	mBasicFlags.Clear();
	mCode.Release();

	if (code.IsValid())
	{
		mHasErrors = false;
		mCheckForSource = false;
		mCode.SerializeFrom(code);

		if (code.Contains("R5_surface", true))
		{
			mBasicFlags.Set(IShader::Flag::Surface, true);
			mBasicFlags.Set(IShader::Flag::Fragment, true);
		}

		if (code.Contains("R5_finalColor", true) ||
			code.Contains("gl_FragColor", true) ||
			code.Contains("gl_FragData", true))
		{
			mBasicFlags.Set(IShader::Flag::Fragment, true);
		}

		if (code.Contains("R5_vertexPosition", true) || code.Contains("gl_Position", true))
		{
			mBasicFlags.Set(IShader::Flag::Vertex, true);
		}

		if (code.Contains("EmitVertex", true))
		{
			mBasicFlags.Set(IShader::Flag::Geometry, true);
		}
	}
	mIsDirty = true;
}

//============================================================================================================
// Force-updates the value of the specified uniform
//============================================================================================================

bool GLSurfaceShader::SetUniform (const String& name, const Uniform& uniform) const
{
	ASSERT(g_lastProgram != 0, "A shader must be activated prior to setting uniforms");
	return (g_lastProgram != 0) && g_lastProgram->SetUniform(name, uniform);
}

//============================================================================================================
// Registers a uniform variable that's updated once per frame
//============================================================================================================

void GLSurfaceShader::RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint group)
{
	FOREACH(i, mUniforms)
	{
		RegisteredUniform& reg = mUniforms[i];

		if (reg.mName == name)
		{
			reg.mDelegate = fnct;
			reg.mGroup = group;

			FOREACH(b, mPrograms)
			{
				GLShaderProgram* pro = mPrograms[b];
				pro->mUniforms.Register(name, fnct, group);
			}
			return;
		}
	}

	// Create a local entry that will be used with new programs
	RegisteredUniform& reg = mUniforms.Expand();
	reg.mName		= name;
	reg.mDelegate	= fnct;
	reg.mGroup		= group;

	// Run through all the existing programs and update their uniform list as well
	FOREACH(b, mPrograms)
	{
		GLShaderProgram* pro = mPrograms[b];
		pro->mUniforms.Register(name, fnct, group);
	}
}