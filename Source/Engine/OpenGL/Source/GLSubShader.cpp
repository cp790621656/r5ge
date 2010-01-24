#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Prints the specified shader log
//============================================================================================================

void R5::PrintDebugLog (const String& log)
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
	for (uint i = 0; i < lines.GetSize(); ++i) System::Log("          - %s", lines[i].GetBuffer());
}

//============================================================================================================
// Common preprocessing function that removes the matched value
//============================================================================================================

uint PreprocessCommon (const String& source,
					   const String& match,
					   String& v0,
					   String& v1,
					   String& v2)
{
	uint length = source.GetLength();
	uint phrase = source.Find(match);

	if (phrase < length)
	{
		String line, vertex, normal, tangent;

		// Extract the entire macroed line
		uint lineEnd = source.GetLine(line, phrase + match.GetLength());

		// Extract the names of the variables
		uint offset = line.GetWord(v0);
		offset = line.GetWord(v1, offset);
		offset = line.GetWord(v2, offset);
		return lineEnd;
	}
	return length;
}

//============================================================================================================
// Macro that adds skinning support. Example implementations:
//============================================================================================================
// // R5_IMPLEMENT_SKINNING vertex
// // R5_IMPLEMENT_SKINNING vertex normal
// // R5_IMPLEMENT_SKINNING vertex normal tangent
//============================================================================================================

bool PreprocessSkinning (String& source)
{
	String left, right, vertex, normal, tangent;

	uint offset = ::PreprocessCommon(source, "R5_IMPLEMENT_SKINNING", vertex, normal, tangent);

	if (vertex.IsValid())
	{
		source.GetString(left, 0, offset);
		source.GetString(right, offset);

		left << "\n{\n";
		left << "mat4 transMat = R5_boneTransforms[int(R5_boneIndex.x)] * R5_boneWeight.x +\n";
		left << "	R5_boneTransforms[int(R5_boneIndex.y)] * R5_boneWeight.y +\n";
		left << "	R5_boneTransforms[int(R5_boneIndex.z)] * R5_boneWeight.z +\n";
		left << "	R5_boneTransforms[int(R5_boneIndex.w)] * R5_boneWeight.w;\n";
		left << "mat3 rotMat = mat3(transMat[0].xyz, transMat[1].xyz, transMat[2].xyz);\n";

		left << vertex;
		left << " = transMat * ";
		left << vertex;
		left << ";\n";

		if (normal.IsValid())
		{
			left << normal;
			left << " = rotMat * ";
			left << normal;
			left << ";\n";
		}

		if (tangent.IsValid())
		{
			left << tangent;
			left << " = rotMat * ";
			left << tangent;
			left << ";\n";
		}

		// Closing bracket
		left << "}\n";

		// Copy the result back into the Source
		source = "uniform mat4 R5_boneTransforms[32];\n";
		source << "attribute vec4 R5_boneWeight;\n";
		source << "attribute vec4 R5_boneIndex;\n";
		source << left;
		source << right;
		return true;
	}
	return false;
}

//============================================================================================================
// Macro that adds pseudo-instancing support. Example implementations:
//============================================================================================================
// // R5_IMPLEMENT_INSTANCING vertex
// // R5_IMPLEMENT_INSTANCING vertex normal
// // R5_IMPLEMENT_INSTANCING vertex normal tangent
//============================================================================================================

bool PreprocessInstancing (String& source)
{
	String left, right, vertex, normal, tangent;

	uint offset = ::PreprocessCommon(source, "R5_IMPLEMENT_INSTANCING", vertex, normal, tangent);

	if (vertex.IsValid())
	{
		source.GetString(left, 0, offset);
		source.GetString(right, offset);

		left << "\n{\n";
		left << "mat4 transMat = mat4(gl_MultiTexCoord2, gl_MultiTexCoord3, gl_MultiTexCoord4, gl_MultiTexCoord5);\n";
		left << "mat3 rotMat = mat3(gl_MultiTexCoord2.xyz, gl_MultiTexCoord3.xyz, gl_MultiTexCoord4.xyz);\n";
		
		left << vertex;
		left << " = transMat * ";
		left << vertex;
		left << ";\n";
		
		if (normal.IsValid())
		{
			left << normal;
			left << " = rotMat * ";
			left << normal;
			left << ";\n";
		}

		if (tangent.IsValid())
		{
			left << tangent;
			left << " = rotMat * ";
			left << tangent;
			left << ";\n";
		}

		// Closing bracket
		left << "}\n";

		// Copy the result back into the Source
		source = left;
		source << right;
		return true;
	}
	return false;
}

//============================================================================================================
// Macro that adds billboard cloud transform functionality.
//============================================================================================================
// // R5_IMPLEMENT_BILLBOARDING vertex
// // R5_IMPLEMENT_BILLBOARDING vertex normal
// // R5_IMPLEMENT_BILLBOARDING vertex normal tangent
//============================================================================================================

bool PreprocessBillboarding (String& source)
{
	String left, right, vertex, normal, tangent;

	uint offset = ::PreprocessCommon(source, "R5_IMPLEMENT_BILLBOARDING", vertex, normal, tangent);

	if (vertex.IsValid())
	{
		source.GetString(left, 0, offset);
		source.GetString(right, offset);

		// View-space offset is calculated based on texture coordinates, enlarged by the size (texCoord's Z)
		left << "\n{\n";
		left << "vec3 offset = gl_MultiTexCoord0.xyz;\n";
	    left << "offset.xy = (offset.xy * 2.0 - 1.0) * offset.z;\n";
		left << "offset.z *= 0.25;\n";
		
		// Calculate the view-transformed vertex
	    left << vertex;
		left << " = gl_ModelViewMatrix * ";
		left << vertex;
		left << ";\n";

		// Apply the view-space offset
		left << vertex;
		left << ".xyz += offset;\n";
		
		if (normal.IsValid())
		{
			left << "vec3 diff = gl_Vertex.xyz - R5_origin;\n";
			left << normal;
			left << " = normalize(gl_NormalMatrix * diff);\n";

			if (tangent.IsValid())
			{
				left << tangent;
				left << " = normalize(gl_NormalMatrix * vec3(diff.y, -diff.x, 0.0));\n";
			}
		}

		// Closing bracket
		left << "}\n";

		// Copy the result back into the Source
		source = "uniform vec3 R5_origin;\n";
		source << left;
		source << right;
		return true;
	}
	return false;
}

//============================================================================================================
// Preprocess all dependencies
//============================================================================================================
// // R5_INCLUDE Deferred/D.vert
// // R5_INCLUDE Deferred/Hello World.frag
//============================================================================================================

void PreprocessDependencies (String& source, Array<String>& dependencies)
{
	String match ("R5_INCLUDE ");
	uint offset = 0, length = source.GetLength();
	
	while (length > (offset = source.Find(match, true, offset)))
	{
		offset = source.GetLine(dependencies.Expand(), offset + match.GetLength());
	}
}

//============================================================================================================
// Delete the OpenGL shader when the SubShader is destroyed
//============================================================================================================

GLSubShader::~GLSubShader()
{
	if (mGLID != -1)
	{
		glDeleteShader(mGLID);
		mGLID = 0;
	}
}

//============================================================================================================
// Preprocess the shader's source code
//============================================================================================================

void GLSubShader::Preprocess()
{
	// Figure out what type of shader this is
	if (mCode.Contains("gl_FragData") || mCode.Contains("gl_FragColor")) mType = Type::Fragment;
	else if (mCode.Contains("gl_Position")) mType = Type::Vertex;

	// Preprocess all macros
	if (mType == Type::Vertex)
	{
		if (::PreprocessSkinning(mCode))		mFlags.Set(IShader::Flag::Skinned,		true);
		if (::PreprocessInstancing(mCode))		mFlags.Set(IShader::Flag::Instanced,	true);
		if (::PreprocessBillboarding(mCode))	mFlags.Set(IShader::Flag::Billboarded,	true);
	}

	// Preprocess all dependencies
	Array<String> list;
	::PreprocessDependencies(mCode, list);

	if (list.IsValid())
	{
		for (uint i = 0; i < list.GetSize(); ++i)
		{
			// Try to find the shader as-is
			GLSubShader* sub = mGraphics->GetSubShader(list[i], mType, false);

			if (sub != 0)
			{
				// Shader entry found -- use it
				mDependencies.Expand() = sub;
			}
			else
			{
				// Create a new sub-shader entry and add it to the list of dependencies
				GLSubShader* sub = mGraphics->GetSubShader(list[i], mType, true);
				mDependencies.Expand() = sub;
#ifdef _DEBUG
				if (sub->mCode.IsEmpty())
				{
					String debug ("Unable to locate '");
					debug << list[i];
					debug << "'!";
					ASSERT(false, debug.GetBuffer());
				}
#endif
			}
		}
	}
}

//============================================================================================================
// Adds its own dependencies and dependencies of dependencies to the list
//============================================================================================================

void GLSubShader::AppendDependenciesTo (Array<GLSubShader*>& list)
{
	for (uint i = mDependencies.GetSize(); i > 0; )
	{
		GLSubShader* sub = mDependencies[--i];

		if (!list.Contains(sub))
		{
			list.Expand() = sub;
			sub->AppendDependenciesTo(list);
		}
	}
}

//============================================================================================================
// Compile the shader
//============================================================================================================

bool GLSubShader::_Compile()
{
	ASSERT(mGLID == -1, "Trying to compile a valid shader?");
	uint type = (mType == Type::Fragment) ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;

	// Create the shader
	mGLID = glCreateShader(type);
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

		if (logLength > 0)
		{
			log.Resize(logLength);
			int charsWritten (0);
			glGetShaderInfoLog (mGLID, logLength, &charsWritten, (char*)log.GetBuffer());
		}

		if (retVal == GL_TRUE)
		{
			System::Log( "[SHADER]  '%s' has compiled successfully", mSource.GetBuffer() );
		}
		else
		{
			System::Log( "[SHADER]  '%s' has FAILED to compile!", mSource.GetBuffer() );

			// Print the debug log if there is something to print
			R5::PrintDebugLog(log);

#ifdef _DEBUG
			// Trigger an assert
			String errMsg ("Failed to compile '");
			errMsg << mSource;
			errMsg << "'!";
			ASSERT(false, errMsg.GetBuffer());
#endif
			// Delete the shader and release the code, making this sub-shader invalid
			glDeleteShader(mGLID);
			mGLID = -1;
			mCode.Clear();
			CHECK_GL_ERROR;
		}
	}
	return (retVal == GL_TRUE);
}