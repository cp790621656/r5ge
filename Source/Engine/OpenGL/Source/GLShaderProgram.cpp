#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Keep track of the currently active program
//============================================================================================================

uint g_activeProgramID = 0;
bool g_fillUniformsList = 0;

//============================================================================================================

GLShaderProgram::~GLShaderProgram()
{
	if (mGLID != 0 && g_activeProgramID == mGLID)
	{
		glUseProgram(0);
	}
}

//============================================================================================================
// Activate the shader program
//============================================================================================================

bool GLShaderProgram::Activate()
{
	if (mIsDirty)
	{
		mIsDirty = false;
		Detach();
		Attach();
	}

	if (g_activeProgramID != mGLID)
	{
		g_activeProgramID = mGLID;
		glUseProgram(mGLID);
	}
	return (g_activeProgramID != 0);
}

//============================================================================================================
// Deactivate the shader program
//============================================================================================================

void GLShaderProgram::Deactivate()
{
	if (g_activeProgramID != 0)
	{
		g_activeProgramID = 0;
		glUseProgram(0);
	}
}

//============================================================================================================
// Detach all shaders from the program
//============================================================================================================

void GLShaderProgram::Detach()
{
	if (mGLID != 0)
	{
		if (mVert != 0 && mVert->mGLID != 0) glDetachShader(mGLID, mVert->mGLID);
		if (mFrag != 0 && mFrag->mGLID != 0) glDetachShader(mGLID, mFrag->mGLID);
		if (mGeom != 0 && mGeom->mGLID != 0) glDetachShader(mGLID, mGeom->mGLID);
		CHECK_GL_ERROR;
	}
}

//============================================================================================================
// Attach all shaders to the program
//============================================================================================================

bool GLShaderProgram::Attach()
{
	if (mGLID == 0)
	{
		mGLID = glCreateProgram();
		ASSERT( mGLID != 0, glGetErrorString() );
		CHECK_GL_ERROR;
	}

	mFlags.Clear();

	if (mVert != 0 && mVert->IsValid())
	{
		mFlags.Set(mVert->mFlags, true);
		glAttachShader(mGLID, mVert->mGLID);
		CHECK_GL_ERROR;
	}

	if (mFrag != 0 && mFrag->IsValid())
	{
		mFlags.Set(mFrag->mFlags, true);
		glAttachShader(mGLID, mFrag->mGLID);
		CHECK_GL_ERROR;
	}

	if (mGeom != 0 && mGeom->IsValid())
	{
		mFlags.Set(mGeom->mFlags, true);
		glAttachShader(mGLID, mGeom->mGLID);
		CHECK_GL_ERROR;
	}

	// Bind all attributes
	mUniforms.BindAttributes(mGLID, (mVert != 0) ? mVert->GetCode() : String());

	// Link the program
	glLinkProgram(mGLID);
	CHECK_GL_ERROR;

	// Get the linking status
	int retVal (0);
	glGetProgramiv(mGLID, GL_LINK_STATUS, &retVal);

	if (retVal == GL_TRUE)
	{
#ifdef _DEBUG
		LogLinkerStatus(true);
#endif
		glUseProgram(g_activeProgramID = mGLID);
		mUniforms.BindTextureUnits(mGLID);
		CHECK_GL_ERROR;
		return true;
	}
	else
	{
		// Log the linker messages
		LogLinkerStatus(false);

		// Deactivate the previous program
		if (g_activeProgramID != 0) glUseProgram(g_activeProgramID = 0);

		// Delete this program
		if (mGLID != 0)
		{
			Detach();
			glDeleteProgram(mGLID);
			CHECK_GL_ERROR;
			mGLID = 0;
		}
		return false;
	}
}

//============================================================================================================
// Logs any information found in the linker log
//============================================================================================================

void GLShaderProgram::LogLinkerStatus (bool success)
{
	String log;
	int logLength (0);
	glGetProgramiv(mGLID, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0)
	{
		log.Resize(logLength);
		int charsWritten (0);
		glGetProgramInfoLog(mGLID, logLength, &charsWritten, (char*)log.GetBuffer());
	}

	System::Log("[SHADER]  Linker log:");
	Array<String> lines;
	R5::CreateDebugLog(lines, log, "");

	// Print the debug log if there is something to print
	if (lines.IsValid())
	{
		FOREACH(i, lines)
		{
			if (lines[i].Contains("BindAttributeLocation") ||
				lines[i].Contains("unused")) continue;

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
	}

	if (success)
	{
		// List the program's common supported features
		if (mFlags.Get(IShader::Flag::Surface))		System::Log("          - Surface shader");
		if (mFlags.Get(IShader::Flag::Deferred))	System::Log("          - Deferred approach");
		if (mFlags.Get(IShader::Flag::Lit))			System::Log("          - Supports lighting");
		if (mFlags.Get(IShader::Flag::Shadowed))	System::Log("          - Supports shadows");
		if (mFlags.Get(IShader::Flag::Skinned))		System::Log("          - Supports skinning");
	}
	else
	{
#ifdef _DEBUG
		String errMsg ("Failed to link shader program!");

		FOREACH(i, lines)
		{
			errMsg << "\n\n";
			errMsg << lines[i];
		}
		ASSERT(false, errMsg.GetBuffer());
#endif
	}
	System::FlushLog();
}

//============================================================================================================
// Checks the source code of attached shader components to see if the specified text exists
//============================================================================================================

bool GLShaderProgram::Contains (const String& text) const
{
	if (mVert != 0 && mVert->mCode.Contains(text)) return true;
	if (mFrag != 0 && mFrag->mCode.Contains(text)) return true;
	if (mGeom != 0 && mGeom->mCode.Contains(text)) return true;
	return false;
}