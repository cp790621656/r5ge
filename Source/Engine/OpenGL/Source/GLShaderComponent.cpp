#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Prints the specified shader log
//============================================================================================================

void R5::CreateDebugLog (Array<String>& out, const String& log, const String& code)
{
	Array<String> lines;
	String debug(log), left, right;

	while (debug.Split(left, '\n', right) || debug.Split(left, '.',  right))
	{
		// Skip the space
		lines.Expand() = (left[0] == ' ') ? &left[1] : left.GetBuffer();
		if (right.IsValid()) debug = (right[0] == ' ') ? &right[1] : right.GetBuffer();
		else debug.Clear();
	}

	if (debug.IsValid()) lines.Expand() = debug;

	// We want to run through all debug lines and write them down
	for (uint i = 0; i < lines.GetSize(); ++i)
	{
		const String& line (lines[i]);
		out.Expand() = line;

		// If we have source code to work with, let's find the lines the errors are referencing
		if (code.IsValid())
		{
			uint col (0xFFFFFFFF), row (0xFFFFFFFF);

			if (line.BeginsWith("ERROR: ") && line.GetLength() > 7)
			{
				// ATI error syntax
				sscanf(line.GetBuffer() + 7, "%u:%u", &col, &row);
			}
			else
			{
				// NVidia error syntax
				sscanf(line.GetBuffer(), "%u (%u)", &col, &row);
			}

			if (row != 0xFFFFFFFF)
			{
				String newLine;
				newLine = "\n";

				// Run through the lines and find the one we've encountered an issue with
				for (uint b = 0, offset = 0; offset < code.GetLength(); ++b)
				{
					if (b + 1 == row)
					{
						// Get this line
						String errorLine;
						code.GetLine(errorLine, offset);
						if (errorLine.IsValid()) out.Expand() = errorLine.GetBuffer();
						break;
					}
					else
					{
						// Move on to the next line
						offset = code.Find(newLine, true, offset) + 1;
					}
				}
			}
		}
	}
}

//============================================================================================================
// Only the GLGraphics class should be creating new shaders
//============================================================================================================

GLShaderComponent::GLShaderComponent (const String& name) :
	mName		(name),
	mGLID		(0),
	mIsDirty	(false) {}

//============================================================================================================
// Release the shader
//============================================================================================================

void GLShaderComponent::_Release()
{
	mFlags.Clear();

	if (mGLID != 0)
	{
		glDeleteShader(mGLID);
		CHECK_GL_ERROR;
		mGLID = 0;
	}
}

//============================================================================================================
// Compile the shader
//============================================================================================================

bool GLShaderComponent::_Compile()
{
	if (!mFlags.Get(IShader::Flag::Vertex | IShader::Flag::Fragment | IShader::Flag::Geometry))
	{
		ASSERT(false, "Compiling an invalid shader?");
		return false;
	}

	uint type = GL_VERTEX_SHADER;
	if		(mFlags.Get(IShader::Flag::Fragment)) type = GL_FRAGMENT_SHADER;
	else if (mFlags.Get(IShader::Flag::Geometry)) type = GL_GEOMETRY_SHADER;

	// Create the shader
	if (mGLID == 0) mGLID = glCreateShader(type);
	ASSERT( mGLID != 0, glGetErrorString() );

	// Set the shader source
	const char* src = mCode.GetBuffer();
	glShaderSource(mGLID, 1, &src, 0);
	CHECK_GL_ERROR;

	// Compile the shader
	glCompileShader(mGLID);
	CHECK_GL_ERROR;

	// Get the compile status
	int retVal (0);
	glGetShaderiv(mGLID, GL_COMPILE_STATUS, &retVal);

#ifndef _DEBUG
	if (retVal != GL_TRUE)
#endif
	{
		// Log any comments
		String log;
		int logLength (0);
		glGetShaderiv (mGLID, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 1)
		{
			log.Resize(logLength);
			int charsWritten (0);
			glGetShaderInfoLog (mGLID, logLength, &charsWritten, (char*)log.GetBuffer());
		}

		Array<String> lines;
		R5::CreateDebugLog(lines, log, mCode);

		if (retVal == GL_TRUE)
		{
			const char* desc = "Vertex";
			if (mFlags.Get(IShader::Flag::Fragment)) desc = "Fragment";
			else if (mFlags.Get(IShader::Flag::Geometry)) desc = "Geometry";
			System::Log( "[SHADER]  '%s' has compiled successfully (%s)", mName.GetBuffer(), desc);
		}
		else
		{
			System::Log( "[SHADER]  '%s' has FAILED to compile!", mName.GetBuffer() );
		}

		if (retVal != GL_TRUE)
		{
#ifdef _DEBUG

			System::Log("==================================== CODE =====================================");
			System::Log(mCode.GetBuffer());
			System::Log("===================================== END =====================================");
			System::FlushLog();

			// Trigger an assert
			String errMsg ("Failed to compile '");
			errMsg << mName;
			errMsg << "'!";
			
			FOREACH(i, lines)
			{
				errMsg << "\n\n";
				errMsg << lines[i];
			}
			ASSERT(false, errMsg.GetBuffer());
#endif
			// Delete the shader and release the code, making this sub-shader invalid
			glDeleteShader(mGLID);
			mGLID = 0;
			mCode.Clear();
			CHECK_GL_ERROR;
		}

		// Print the debug log if there is something to print
		if (lines.IsValid())
		{
			FOREACH(i, lines)
			{
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
	}
	return (retVal == GL_TRUE);
}

//============================================================================================================
// Changes the code for the current shader
//============================================================================================================

void GLShaderComponent::SetCode (const String& code, const Flags& flags)
{
	if (mCode != code)
	{
		mIsDirty = true;
		mCode	 = code;
		mFlags	 = flags;
	}
}

//============================================================================================================
// Validates the shader, compiling it if necessary
//============================================================================================================

bool GLShaderComponent::IsValid()
{
	if (!mIsDirty)
	{
		if (mGLID != 0) return true;
		if (mCode.IsEmpty()) return false;
	}
	mIsDirty = false;
	return _Compile();
}