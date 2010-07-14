#include "../Include/_All.h"
#include "../Include/_OpenGL.h"

// Built-in shaders
#include "../Shaders/Deferred.h"
#include "../Shaders/Lights.h"
#include "../Shaders/SSAO.h"
#include "../Shaders/Blur.h"
#include "../Shaders/Bloom.h"
#include "../Shaders/DOF.h"
#include "../Shaders/Shadow.h"

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
		left << "offset *= R5_worldScale;\n";
		
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
			left << "vec3 diff = gl_Vertex.xyz - R5_origin.xyz;\n";
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
		source << "uniform float R5_worldScale;\n";
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
// Only the GLGraphics class should be creating new shaders
//============================================================================================================

GLSubShader::GLSubShader (GLGraphics* graphics, const String& name, byte type) :
	mGraphics	(graphics),
	mType		(type),
	mGLID		(0),
	mIsDirty	(false)
{
	mName = name;
}

//============================================================================================================
// INTERNAL: Initialize the sub-shader, try to load its source code if possible
//============================================================================================================

void GLSubShader::_Init()
{
	// Shaders that begin with [R5] are built-in
	if (mName.BeginsWith("[R5]"))
	{
		if		(mName == "[R5] Combine Deferred")		mCode = g_combineDeferred;
		else if (mName == "[R5] Horizontal Blur")		mCode = g_blurH;
		else if (mName == "[R5] Vertical Blur")			mCode = g_blurV;
		else if (mName == "[R5] Bloom Blur")			mCode = g_blurBloom;
		else if (mName == "[R5] Combine Bloom")			mCode = g_combineBloom;
		else if (mName == "[R5] Depth of Field")		mCode = g_dof;
		else if (mName == "[R5] Sample SSAO")			mCode = g_ssaoSample;
		else if (mName == "[R5] Horizontal Depth-Respecting Blur")
		{
			mCode  = g_depthRespectingBlur;
			mCode << g_depthRespectingBlurH;
		}
		else if (mName == "[R5] Vertical Depth-Respecting Blur")
		{
			mCode  = g_depthRespectingBlur;
			mCode << g_depthRespectingBlurV;
		}
		else if (mName == "[R5] Horizontal SSAO Blur")
		{
			mCode  = g_ssaoBlur;
			mCode << g_ssaoBlurH;
		}
		else if (mName == "[R5] Vertical SSAO Blur")
		{
			mCode  = g_ssaoBlur;
			mCode << g_ssaoBlurV;
		}
		else if (mName.Contains("Light"))
		{
			// Light shaders are compiled from multiple sources in order to reduce code repetition
			bool ao = mName.Contains("AO");
			bool shadow = mName.Contains("Shadow");

			if (ao)				mCode = g_lightPrefix4;
			else if (shadow)	mCode = g_lightPrefix3;
			else				mCode = g_lightPrefix2;

			mCode << g_lightCommon;

			// Light-specific code
			if (mName.Contains("Directional"))
			{
				mCode << g_lightDirectional;
				mCode << g_lightBody;
				if (shadow) mCode << g_lightShadow;
				mCode << (ao ? g_lightEndDirAO : g_lightEndDir);
			}
			else
			{
				mCode << g_lightPoint;
				mCode << g_lightBody;
				if (shadow) mCode << g_lightShadow;
				mCode << (ao ? g_lightEndPointAO : g_lightEndPoint);
			}
		}
		else if (mName.BeginsWith("[R5] Shadow"))
		{
			// Expected syntax: "[R5] Shadow %u"
			String out;
			mName.GetString(out, 12);
			uint cascades (0);

			// The number of supported shadow cascades must be valid
			if (out >> cascades && cascades < 5 && cascades > 0)
			{
				// Regardless of cascades, the first texture is always camera's depth
				mCode << "uniform sampler2D R5_texture0;\n";

				// Light's depth textures follow
				for (uint i = 0; i < cascades; ++i)
				{
					mCode << "uniform sampler2DShadow R5_texture";
					mCode << (i + 1);
					mCode << ";\n";
				}

				// Next come the shadow matrices
				for (uint i = 0; i < cascades; ++i)
				{
					mCode << "uniform mat4 shadowMatrix";
					mCode << i;
					mCode << ";\n";
				}

				// Next comes the common component
				mCode << g_shadow;

				for (uint i = 0; i < cascades; ++i)
				{
					if (i == 0)
					{
						// First cascade defines the variables
						mCode << "vec4 pos4 = shadowMatrix0 * worldPos;\n";
						mCode << "float shadow = Sample(R5_texture1, pos4.xyz / pos4.w);\n";
					}
					else
					{
						// Set the new shadow matrix
						mCode << "pos4 = shadowMatrix";
						mCode << i;
						mCode << " * worldPos;\n";

						// Sample the texture and choose the smallest value
						mCode << "shadow = min(shadow, Sample(R5_texture";
						mCode << (i + 1);
						mCode << ", pos4.xyz / pos4.w));\n";
					}
				}

				// Set the color and end the function
				mCode << "gl_FragColor = vec4(shadow);\n}";
			}
		}
#ifdef _DEBUG
		else ASSERT(false, "Unrecognized internal shader request");
#endif
	}
	else
	{
		// Try to load the code from a file
		mCode.Load(mName);
	}

	if (mCode.IsValid()) _Preprocess();
}

//============================================================================================================
// Release the shader
//============================================================================================================

void GLSubShader::_Release()
{
	mType = Type::Invalid;

	if (mGLID != 0)
	{
		glDeleteShader(mGLID);
		mGLID = 0;
	}
}

//============================================================================================================
// Preprocess the shader's source code
//============================================================================================================

void GLSubShader::_Preprocess()
{
	mFlags.Clear();
	mDependencies.Clear();

	// Figure out what type of shader this is
	if (mCode.Contains("EndPrimitive();")) mType = Type::Geometry;
	else if (mCode.Contains("gl_FragData") || mCode.Contains("gl_FragColor")) mType = Type::Fragment;
	else if (mCode.Contains("gl_Position")) mType = Type::Vertex;

	// Preprocess all macros
	if (mType == Type::Vertex)
	{
		if (::PreprocessSkinning(mCode))		mFlags.Set(IShader::Flag::Skinned,		true);
		if (::PreprocessInstancing(mCode))		mFlags.Set(IShader::Flag::Instanced,	true);
		if (::PreprocessBillboarding(mCode))	mFlags.Set(IShader::Flag::Billboarded,	true);
		if (mCode.Contains("R5_worldScale"))	mFlags.Set(IShader::Flag::WorldScale,	true);
	}

	// Preprocess all dependencies
	Array<String> list;
	::PreprocessDependencies(mCode, list);

	if (list.IsValid())
	{
		for (uint i = 0; i < list.GetSize(); ++i)
		{
			// Try to find the shader as-is
			GLSubShader* sub = mGraphics->GetGLSubShader(list[i], false, mType);

			if (sub != 0)
			{
				// Shader entry found -- use it
				mDependencies.Expand() = sub;
			}
			else
			{
				// Create a new sub-shader entry and add it to the list of dependencies
				GLSubShader* sub = mGraphics->GetGLSubShader(list[i], true, mType);
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
// Compile the shader
//============================================================================================================

bool GLSubShader::_Compile()
{
	ASSERT(mType != Type::Invalid, "Compiling an invalid shader type?");

	if (mType == Type::Invalid) return false;

	uint type = GL_VERTEX_SHADER;
	if		(mType == Type::Fragment) type = GL_FRAGMENT_SHADER;
	else if (mType == Type::Geometry) type = GL_GEOMETRY_SHADER;

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

		if (retVal == GL_TRUE)
		{
			System::Log( "[SHADER]  '%s' has compiled successfully", mName.GetBuffer() );
		}
		else
		{
			System::Log( "[SHADER]  '%s' has FAILED to compile!", mName.GetBuffer() );

			// Print the debug log if there is something to print
			Array<String> lines;
			R5::CreateDebugLog(lines, log, mCode);

			if (lines.IsValid())
			{
				FOREACH(i, lines)
				{
					System::Log("          - %s", lines[i].GetBuffer());
				}
				System::FlushLog();
			}

#ifdef _DEBUG
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
		//System::Log(mCode);
	}
	return (retVal == GL_TRUE);
}

//============================================================================================================
// Changes the code for the current shader
//============================================================================================================

void GLSubShader::SetCode (const String& code, bool notifyShaders)
{
	if (mCode != code)
	{
		mIsDirty = true;
		mCode	 = code;

		// Preprocess the source code
		_Preprocess();

		if (notifyShaders)
		{
			// Retrieve all current shaders from the graphics manager
			GLGraphics::Shaders& shaders = mGraphics->GetAllShaders();

			// Run through all shaders and mark those using this shader as needing to be relinked
			for (uint i = shaders.GetSize(); i > 0; )
			{
				GLShader* shader = (GLShader*)shaders[--i];
				if (shader->IsUsingSubShader(this)) shader->SetDirty();
			}
		}
	}
}

//============================================================================================================
// Validates the shader, compiling it if necessary
//============================================================================================================

bool GLSubShader::IsValid()
{
	if (!mIsDirty)
	{
		if (mGLID != 0) return true;
		if (mCode.IsEmpty()) return false;
	}
	mIsDirty = false;
	return _Compile();
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

			// NOTE: Disabling nested includes for now (Feb 16, 2010)
			//sub->AppendDependenciesTo(list);
		}
	}
}