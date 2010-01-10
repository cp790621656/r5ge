#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// To keep track of currently active program
//============================================================================================================

uint g_activeProgram = 0;
uint g_activeLightCount = 0;

//============================================================================================================
// Retrieves the information log for the specified shader
//============================================================================================================

void GetShaderInfoLog(uint shader, String& out)
{
	out.Clear();

	int logLength (0);

	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0)
	{
		out.Resize(logLength);
		int charsWritten (0);
		glGetShaderInfoLog (shader, logLength, &charsWritten, (char*)out.GetBuffer());
	}
}

//============================================================================================================
// Retrieves the information log for the specified program
//============================================================================================================

void GetProgramInfoLog(uint shader, String& out)
{
	out.Clear();

	int logLength (0);

	glGetProgramiv (shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0)
	{
		out.Resize(logLength);
		int charsWritten (0);
		glGetProgramInfoLog (shader, logLength, &charsWritten, (char*)out.GetBuffer());
	}
}

//============================================================================================================
// Splits the info log string into separate lines
//============================================================================================================

void SplitInfoLog(const String& infoLog, Array<String>& lines)
{
	String debug(infoLog), left, right;

	while ( debug.Split(left, '\n', right) ||
			debug.Split(left, '.',  right) )
	{
		// Skip the space
		lines.Expand() = (left[0] == ' ') ? &left[1] : left.GetBuffer();

		if ( right.IsValid() )
		{
			debug = (right[0] == ' ') ? &right[1] : right.GetBuffer();
		}
		else debug.Clear();
	}

	if ( debug.IsValid() ) lines.Expand() = debug;
}

//============================================================================================================
// Prints the shader log information
//============================================================================================================

void PrintInfoLog(const String& infoLog)
{
	Array<String> lines;
	SplitInfoLog(infoLog, lines);

	for (uint i = 0; i < lines.GetSize(); ++i)
	{
		System::Log("          - %s", lines[i].GetBuffer());
	}
}

//============================================================================================================
// Compiles the shader and retrieves whether the operation was successful, with an error log
//============================================================================================================

bool CompileShader (uint shader, String& out)
{
	int retVal (0);
	glCompileShader(shader);
	CHECK_GL_ERROR;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &retVal);
	GetShaderInfoLog(shader, out);
	CHECK_GL_ERROR;
	return (retVal == GL_TRUE);
}

//============================================================================================================
// Links the program together and returns whether the operation was successful, with an error log
//============================================================================================================

bool LinkProgram (uint program, String& out)
{
	int retVal (0);

	if (program != 0)
	{
		// R5 shader attributes should be automatically bound to be associated with their IGraphics counterparts
		glBindAttribLocation(program, IGraphics::Attribute::Position,		"R5_position");
		glBindAttribLocation(program, IGraphics::Attribute::Tangent,		"R5_tangent");
		glBindAttribLocation(program, IGraphics::Attribute::Normal,			"R5_normal");
		glBindAttribLocation(program, IGraphics::Attribute::Color,			"R5_color");
		glBindAttribLocation(program, IGraphics::Attribute::SecondaryColor,	"R5_secondaryColor");
		glBindAttribLocation(program, IGraphics::Attribute::FogCoord,		"R5_fogCoord");
		glBindAttribLocation(program, IGraphics::Attribute::BoneWeight,		"R5_boneWeight");
		glBindAttribLocation(program, IGraphics::Attribute::BoneIndex,		"R5_boneIndex");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord0,		"R5_texCoord0");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord1,		"R5_texCoord1");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord2,		"R5_texCoord2");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord3,		"R5_texCoord3");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord4,		"R5_texCoord4");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord5,		"R5_texCoord5");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord6,		"R5_texCoord6");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord7,		"R5_texCoord7");

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &retVal);
		GetProgramInfoLog(program, out);
	}
	return (retVal == GL_TRUE);
}

//============================================================================================================
// Creates a shader of specified type using the specified source
//============================================================================================================

uint CreateShader (const String& name, const String& code, uint type)
{
	uint shader = glCreateShader(type);
	ASSERT( shader != 0, glGetErrorString() );

	// Set the shader source
	const char* src = code.GetBuffer();
	glShaderSource(shader, 1, &src, 0);
	CHECK_GL_ERROR;

	// Compile the shader
	String debug;
	if ( CompileShader(shader, debug) )
	{
#ifdef _DEBUG
		System::Log( "          - Compiled '%s'", name.GetBuffer());
#endif
	}
	else
	{
		// Something went wrong -- log the error
#ifdef _DEBUG
		System::Log( "          - FAILED to compile '%s'", name.GetBuffer() );
#else
		System::Log( "[SHADER]  FAILED to compile '%s'", name.GetBuffer() );
#endif
		PrintInfoLog(debug);
		glDeleteShader(shader);
		shader = 0;
		WARNING("Shader compilation failure");
	}
	return shader;
}

//============================================================================================================
// Attach a set of shaders to the program, then link them together
//============================================================================================================

bool AttachAndLink( String& debug, uint& program, uint& vertex, uint& pixel )
{
	if ( program == 0 )
	{
		program = glCreateProgram();
		ASSERT( program != 0, glGetErrorString() );
		CHECK_GL_ERROR;
	}

	if (vertex) glAttachShader(program, vertex);
	if (pixel)  glAttachShader(program, pixel);
	CHECK_GL_ERROR;

	if ( LinkProgram(program, debug) )
	{
		glUseProgram( g_activeProgram = program );
		CHECK_GL_ERROR;
		return true;
	}

	glDeleteProgram(program);
	program = 0;
	return false;
}

//============================================================================================================
// Detach and delete a specific shader
//============================================================================================================

inline void DetachAndDelete(uint program, uint& shader)
{
	if (shader != 0)
	{
		if (program != 0) glDetachShader(program, shader);
		glDeleteShader(shader);
		shader = 0;
	}
}

//============================================================================================================
// Detach the specified shader
//============================================================================================================

inline void DetachShader (uint program, uint shader)
{
	if (program != 0 && shader != 0)
	{
		glDetachShader(program, shader);
	}
}

//============================================================================================================
// Finds a shader file given the filename and extension
//============================================================================================================

String FindShader (const String& file, const char* extension)
{
	String path (file);

	if (System::FileExists(path)) return path;

	path = file + extension;
	if (System::FileExists(path)) return path;

	if (!path.BeginsWith("Shaders/")) path = String("Shaders/") + path;
	if (System::FileExists(path)) return path;

	path = String("Shaders/") + System::GetFilenameFromPath(file);
	if (System::FileExists(path)) return path;

	path << extension;
	if (System::FileExists(path)) return path;

	path = "Resources/" + path;
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
			System::Log("          - Found constant uniform '%s' [%u]", name, loc);
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
// Updates a uniform entry
//============================================================================================================

bool UpdateUniform (uint glID, const Uniform& uni)
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
// Determines whether the shader's source will need pre-processing
//============================================================================================================

inline bool NeedsLightPreprocessing (const String& source)
{
	return source.Find("R5_FOR_EACH_LIGHT") != source.GetLength();
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

	uint offset = PreprocessCommon(source, "R5_IMPLEMENT_SKINNING", vertex, normal, tangent);

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

	uint offset = PreprocessCommon(source, "R5_IMPLEMENT_INSTANCING", vertex, normal, tangent);

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

	uint offset = PreprocessCommon(source, "R5_IMPLEMENT_BILLBOARDING", vertex, normal, tangent);

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
// Unwraps the 'for' loops then compiles the shader (fix for shoddy GLSL support on Intel cards)
//============================================================================================================

bool PreprocessLights (const String& name, GLShader::ShaderEntry& entry, GLShader::ShaderInfo& info, uint type, uint lights)
{
	if (entry.mGlID == 0 && entry.mStatus != GLShader::ShaderEntry::CompileStatus::Error)
	{
		if (info.mSource.IsValid())
		{
			if (info.mSpecial)
			{
				uint offset (0);
				String source (info.mSource);

				while ( (offset = source.Find("R5_FOR_EACH_LIGHT", true, offset)) != source.GetLength() )
				{
					int braces = 0;
					uint start (source.GetLength());
					uint end = start;

					for (uint i = offset; i < source.GetLength(); ++i)
					{
						char letter = source[i];

						// Opening brace -- potential beginning of the block
						if (letter == '{')
						{
							if (++braces == 1) start = i;
						}
						// Closing brace -- potential end of the block
						else if (letter == '}')
						{
							if (--braces == 0)
							{
								end = i;
								break;
							}
						}
					}

					if ( start == source.GetLength() || end == source.GetLength() )
					{
						entry.mStatus = GLShader::ShaderEntry::CompileStatus::Error;
#ifdef _DEBUG
						System::Log("          - ERROR! Mismatched brackets found while compiling '%s'", name.GetBuffer());
#else
						System::Log("[SHADER]  ERROR! Mismatched brackets found while compiling '%s'", name.GetBuffer());
#endif
						return false;
					}

					// Move up the offset to the end of the block
					offset = end+1;
					String block, left, right, copy;
					source.GetString(left, 0, start);
					source.GetString(block, start, offset);
					source.GetString(right, offset);

					// Beginning of the code
					source = left;

					// Block entries for every light
					for (uint i = 0; i < lights; ++i)
					{
						if (i == 0)
						{
							source << block;
						}
						else
						{
							copy = block;
							String index;
							index.Set("[%u]", i);
							copy.Replace("[0]", index);
							source << copy;
						}
					}

					// Finish by adding the rest of the code
					offset = source.GetLength();
					source << right;
				}

				// Attempt to create the shader
				entry.mGlID = ::CreateShader(name, source, type == GLShader::Type::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
			}
			else // not marked as 'special'
			{
				// Attempt to create the shader
				entry.mGlID = ::CreateShader(name, info.mSource, type == GLShader::Type::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
			}
		}

		// Remember the result
		entry.mStatus = (entry.mGlID == 0) ?
			GLShader::ShaderEntry::CompileStatus::Error :
			GLShader::ShaderEntry::CompileStatus::Success;
	}

	// Return whether the shader was compiled successfully
	return (entry.mStatus == GLShader::ShaderEntry::CompileStatus::Success);
}

//============================================================================================================
// Actual shader class begins here...
//============================================================================================================

GLShader::GLShader (const String& name) :
	mName			(name),
	mIsDirty		(false),
	mSerializable	(false),
	mLast			(g_caps.mMaxLights + 1),
	mUpdateStamp	(0)
{
	mVertexInfo.mPath   = ::FindShader(name, ".vert");
	mFragmentInfo.mPath = ::FindShader(name, ".frag");

	SetSourcePath (mVertexInfo.mPath,	Type::Vertex);
	SetSourcePath (mFragmentInfo.mPath, Type::Fragment);

	// If the path was deduced from the shader's name, don't serialize this shader
	mSerializable = mVertexInfo.mPath.IsEmpty() && mFragmentInfo.mPath.IsEmpty();
}

//============================================================================================================
// Non thread-safe version of Release(), for internal use only
//============================================================================================================

void GLShader::_InternalRelease (bool clearUniforms)
{
	mLast = g_caps.mMaxLights + 1;

	// Detach and release all shaders
	for (uint i = mPrograms.GetSize(); i > 0; )
	{
		ProgramEntry& entry (mPrograms[--i]);
		uint& pid = entry.mProgram.mGlID;

		if (pid != 0)
		{
			if (mVertexInfo.mSpecial || i == 0)		::DetachAndDelete( pid, mPrograms[i].mVertex.mGlID );
			else									::DetachShader   ( pid, mPrograms[0].mVertex.mGlID );

			if (mFragmentInfo.mSpecial || i == 0)	::DetachAndDelete( pid, mPrograms[i].mFragment.mGlID );
			else									::DetachShader   ( pid, mPrograms[0].mFragment.mGlID );

			glDeleteProgram(pid);
			pid = 0;
		}

		// Reset all uniforms back to "not found" state
		Array<UniformEntry>& uniforms (entry.mUniforms);

		uniforms.Lock();
		{
			if (clearUniforms) uniforms.Clear();
			else for (uint i = 0; i < uniforms.GetSize(); ++i)
				uniforms[i].mGlID = -2;
		}
		uniforms.Unlock();
	}
}

//============================================================================================================
// Activates the shader compiled for the specified number of lights
//============================================================================================================

IShader::ActivationResult GLShader::_Activate (uint activeLightCount, bool updateUniforms)
{
	CHECK_GL_ERROR;
	ActivationResult result;

	// Release the shaders if necessary
	if (mIsDirty)
	{
		mIsDirty = false;
		_InternalRelease(false);
	}

	// Ensure that we have the correct number of programs
	uint maxEntries = g_caps.mMaxLights + 1;
	if (mPrograms.GetSize() < maxEntries)
		mPrograms.ExpandTo(maxEntries);

	// Just in case, never exceed the maximum number of lights
	if (activeLightCount > maxEntries)
		activeLightCount = maxEntries;

	// If neither the vertex shader nor the fragment shader require special
	// preprocessing, always use the shader compiled for no lights.
	if (!mVertexInfo.mSpecial && !mFragmentInfo.mSpecial)
		activeLightCount = 0;

	// When shaders fail to link, they decrement the 'mLast' variable, in essence
	// forcing the next activation to try to compile this shader with less lights.
	if (activeLightCount > mLast)
		activeLightCount = mLast;

	// Find the best match in the shader list
	for (uint i = activeLightCount; ; --i)
	{
		ProgramEntry& entry	  ( mPrograms[i] );
		ShaderEntry& program  ( entry.mProgram );
		ShaderEntry& fragment ( mFragmentInfo.mSpecial ? entry.mFragment : mPrograms[0].mFragment );
		ShaderEntry& vertex	  (   mVertexInfo.mSpecial ? entry.mVertex   : mPrograms[0].mVertex   );

		// If the program hasn't been compiled, do it now
		if (program.mStatus == ShaderEntry::CompileStatus::Unknown && program.mGlID == 0)
		{
			result.mReused = false;
			int success (0);

#ifdef _DEBUG
			System::Log("[SHADER]  Compiling '%s' for %u lights", mName.GetBuffer(), i);
#endif

			// Compile the vertex shader
			if (vertex.mStatus == ShaderEntry::CompileStatus::Unknown && mVertexInfo.mSource.IsValid())
			{
				::PreprocessLights(mVertexInfo.mPath.IsValid() ? mVertexInfo.mPath :
					(mName + " (Vertex)"), vertex, mVertexInfo,
					Type::Vertex, mVertexInfo.mSpecial ? i : 0);

				if (vertex.mStatus == ShaderEntry::CompileStatus::Error && vertex.mGlID != 0)
				{
					glDeleteShader(vertex.mGlID);
					vertex.mGlID = 0;
				}
			}
#ifdef _DEBUG
			else System::Log("          - No vertex shader");
#endif

			// Compile the fragment shader
			if (fragment.mStatus == ShaderEntry::CompileStatus::Unknown && mFragmentInfo.mSource.IsValid())
			{
				::PreprocessLights(mFragmentInfo.mPath.IsValid() ? mFragmentInfo.mPath :
					(mName + " (Fragment)"), fragment, mFragmentInfo, Type::Fragment,
					mFragmentInfo.mSpecial ? i : 0);

				if (fragment.mStatus == ShaderEntry::CompileStatus::Error && fragment.mGlID != 0)
				{
					glDeleteShader(fragment.mGlID);
					fragment.mGlID = 0;
				}
			}
#ifdef _DEBUG
			else System::Log("          - No fragment shader");
#endif

			// Add up the results
			if		(vertex.mStatus		== ShaderEntry::CompileStatus::Success)	++success;
			else if (vertex.mStatus		== ShaderEntry::CompileStatus::Error)	success -= 100;
			if		(fragment.mStatus	== ShaderEntry::CompileStatus::Success) ++success;
			else if (fragment.mStatus	== ShaderEntry::CompileStatus::Error)	success -= 100;

			// Only continue if there is something to work with
			if (success > 0)
			{
				String debug;

				// Try to link the shaders together -- this step may fail if shaders exceed
				// the allowed resources (number of variables, for example)
				if ( ::AttachAndLink(debug, program.mGlID, vertex.mGlID, fragment.mGlID) )
				{
					updateUniforms = true;
					program.mStatus = ShaderEntry::CompileStatus::Success;

					// Set the values of texture units in the shader
					::SetUniform1i(program.mGlID, "R5_texture0", 0);
					::SetUniform1i(program.mGlID, "R5_texture1", 1);
					::SetUniform1i(program.mGlID, "R5_texture2", 2);
					::SetUniform1i(program.mGlID, "R5_texture3", 3);
					::SetUniform1i(program.mGlID, "R5_texture4", 4);
					::SetUniform1i(program.mGlID, "R5_texture5", 5);
					::SetUniform1i(program.mGlID, "R5_texture6", 6);
					::SetUniform1i(program.mGlID, "R5_texture7", 7);

					CHECK_GL_ERROR;
#ifdef _DEBUG
					System::Log("          - Linked successfully");
					System::Log("          - Linker Log:\n%s", debug.GetBuffer());
#endif
				}
				else // Linking failed -- resources may have been exceeded
				{
					vertex.mStatus   = ShaderEntry::CompileStatus::Error;
					fragment.mStatus = ShaderEntry::CompileStatus::Error;
					program.mStatus  = ShaderEntry::CompileStatus::Error;

					// Delete the shader if it has been preprocessed, otherwise only detach it
					if (  mVertexInfo.mSpecial || i == 0)	::DetachAndDelete( program.mGlID, vertex.mGlID );
					else									::DetachShader	 ( program.mGlID, vertex.mGlID );
					if (mFragmentInfo.mSpecial || i == 0)	::DetachAndDelete( program.mGlID, fragment.mGlID );
					else									::DetachShader	 ( program.mGlID, fragment.mGlID );

#ifdef _DEBUG
					System::Log("          - FAILED to link, will not be used");
					System::Log("          - Linker Log:\n%s", debug.GetBuffer());
#else
					if (i == 0)
					{
						System::Log("[SHADER]  FAILED to link '%s'", mName.GetBuffer());
						System::Log("          - Linker Log:\n%s", debug.GetBuffer());
					}
#endif
					if (mLast > i) mLast = i;
					g_activeProgram = 0;
				}
			}
			else
			{
				// Some compiling error occurred -- exit prematurely
				vertex.mStatus   = ShaderEntry::CompileStatus::Error;
				fragment.mStatus = ShaderEntry::CompileStatus::Error;
				program.mStatus  = ShaderEntry::CompileStatus::Error;

				// Mark the most basic shader as having failed as well
				mPrograms[mLast=0].mProgram.mStatus = ShaderEntry::CompileStatus::Error;

				if (vertex.mGlID)
				{
					glDeleteShader(vertex.mGlID);
					vertex.mGlID = 0;
				}

				if (fragment.mGlID)
				{
					glDeleteShader(fragment.mGlID);
					fragment.mGlID = 0;
				}

				CHECK_GL_ERROR;

				// No need to keep the defective source code
				mVertexInfo.mSource.Release();
				mFragmentInfo.mSource.Release();

				// Compiling errors indicate a problem with the source code,
				// so there is no need to continuing beyond this point.
				g_activeProgram = 0;
				break;
			}
		}

		if (program.mStatus == ShaderEntry::CompileStatus::Success)
		{
			// Remember the number of lights this program was activated for
			g_activeLightCount = activeLightCount;

			// Use the program if it hasn't been used already
			if (g_activeProgram != program.mGlID)
			{
				result.mReused = false;
				updateUniforms = true;
				glUseProgram( g_activeProgram = program.mGlID );
				CHECK_GL_ERROR;
			}

			// Update uniform callbacks every millisecond as well, since the same shader
			// can stay enabled from one frame into the next.
			ulong current = Time::GetMilliseconds();
			if (current - mUpdateStamp > 1) updateUniforms = true;

			// List of registered uniforms
			Array<UniformEntry>& uniforms (entry.mUniforms);

			// Update all registered uniforms
			if (updateUniforms && program.mGlID != 0 && uniforms.GetSize() > 0)
			{
				Uniform uni;
				mUpdateStamp = current;

				for (uint u = uniforms.GetSize(); u > 0; )
				{
					const UniformEntry& entry = uniforms[--u];

					if (entry.mGlID != -1 && entry.mDelegate)
					{
						// Find the uniform if we have not yet tried to find it
						if (entry.mGlID == -2) entry.mGlID = GetUniformID(entry.mName);

						// If the uniform has been found, update it
						if (entry.mGlID != -1)
						{
							uni.mType = Uniform::Type::Invalid;
							entry.mDelegate(entry.mName, uni);
							::UpdateUniform(entry.mGlID, uni);
						}
					}
				}
			}
			// Success -- exit the function
			result.mCount = i;
			return result;
		}

		// If the end of entries has been reached, break out
		if (i == 0) break;
	}

	// If this point was reached, the shader program is invalid
	if (g_activeProgram != 0)
	{
		glUseProgram(g_activeProgram = 0);
		g_activeLightCount = 0;
	}
	return result;
}

//============================================================================================================
// Directly sets the source code for the shader
//============================================================================================================

void GLShader::_SetSourceCode (const String& code, uint type)
{
	mIsDirty = true;
	mSerializable = false;

	if (type == Type::Vertex)
	{
		mVertexInfo.mPath.Clear();
		mVertexInfo.mSource = code;
		mVertexInfo.mSpecial = ::NeedsLightPreprocessing(mVertexInfo.mSource);

		if (::PreprocessSkinning(mVertexInfo.mSource))		SetFlag(Flag::Skinned,	 true);
		if (::PreprocessInstancing(mVertexInfo.mSource))	SetFlag(Flag::Instanced, true);
		if (::PreprocessBillboarding(mVertexInfo.mSource))	SetFlag(Flag::Billboarded, true);
	}
	else
	{
		mFragmentInfo.mPath.Clear();
		mFragmentInfo.mSource = code;
		mFragmentInfo.mSpecial = ::NeedsLightPreprocessing(mFragmentInfo.mSource);
	}
}

//============================================================================================================
// Sets the path where the shader's source code can be found
//============================================================================================================

void GLShader::_SetSourcePath (const String& path, uint type)
{
	mIsDirty = true;

	String text;
	if (text.Load(path)) _SetSourceCode(text, type);

	// Serialization should only happen if the paths are valid
	mSerializable = mVertexInfo.mPath.IsValid() || mFragmentInfo.mPath.IsValid();
}

//============================================================================================================
// Releases the source for the shader and marks it as dirty so it's released next frame
//============================================================================================================

void GLShader::Release()
{
	mLock.Lock();
	{
		mVertexInfo.Clear();
		mFragmentInfo.Clear();
		mIsDirty = true;
	}
	mLock.Unlock();
}

//============================================================================================================
// Safely activates the shader
//============================================================================================================

IShader::ActivationResult GLShader::Activate (uint activeLightCount, bool forceUpdateUniforms) const
{
	mLock.Lock();
	ActivationResult retVal = (const_cast<GLShader*>(this))->_Activate(activeLightCount, forceUpdateUniforms);
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Deactivates the active shader
//============================================================================================================

void GLShader::Deactivate() const
{
	if ( g_activeProgram != 0 )
	{
		glUseProgram( g_activeProgram = 0 );
		g_activeLightCount = 0;
	}
}

//============================================================================================================
// Registers a uniform variable
//============================================================================================================

void GLShader::RegisterUniform (const String& name, const SetUniformDelegate& fnct)
{
	uint maxEntries = g_caps.mMaxLights + 1;
	if (mPrograms.GetSize() < maxEntries)
		mPrograms.ExpandTo(maxEntries);

	for (uint i = 0; i < mPrograms.GetSize(); ++i)
	{
		mPrograms[i].RegisterUniform(name, fnct);
	}
}

//============================================================================================================

void GLShader::ProgramEntry::RegisterUniform (const String& name, const SetUniformDelegate& fnct)
{
	mUniforms.Lock();
	{
		for (uint i = 0; i < mUniforms.GetSize(); ++i)
		{
			UniformEntry& entry = mUniforms[i];

			if (entry.mName == name)
			{
				entry.mDelegate = fnct;
				mUniforms.Unlock();
				return;
			}
		}

		UniformEntry& entry = mUniforms.Expand();
		entry.mName			= name;
		entry.mDelegate		= fnct;
	}
	mUniforms.Unlock();
}

//============================================================================================================
// Force-updates the value of the specified uniform
//============================================================================================================

bool GLShader::UpdateUniform (const String& name, const Uniform& uniform) const
{
	if (g_activeProgram != 0)
	{
		if (mPrograms[g_activeLightCount].mProgram.mGlID == g_activeProgram)
		{
			return mPrograms[g_activeLightCount].UpdateUniform(name, uniform);
		}
	}
	return false;
}

//============================================================================================================

bool GLShader::ProgramEntry::UpdateUniform (const String& name, const Uniform& uniform) const
{
	mUniforms.Lock();
	{
		for (uint i = mUniforms.GetSize(); i > 0; )
		{
			const UniformEntry& entry = mUniforms[--i];

			if (entry.mName == name)
			{
				if (entry.mGlID == -2) entry.mGlID = GetUniformID(name);
				if (entry.mGlID != -1)
				{
					::UpdateUniform(entry.mGlID, uniform);
					mUniforms.Unlock();
					return true;
				}
			}
		}

		// This must be a new entry -- try to add it
		{
			UniformEntry& entry = mUniforms.Expand();
			entry.mName = name;
			entry.mGlID = GetUniformID(name);
			
			if (entry.mGlID != -1)
			{
				::UpdateUniform(entry.mGlID, uniform);
				mUniforms.Unlock();
				return true;
			}
		}
	}
	mUniforms.Unlock();
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool GLShader::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	if (!IsValid() || forceUpdate)
	{
		mLock.Lock();
		{
			mSerializable = true;

			for (uint i = 0; i < root.mChildren.GetSize(); ++i)
			{
				const TreeNode& node  = root.mChildren[i];
				const String&	tag   = node.mTag;
				const Variable&	value = node.mValue;

				if (tag == "Serializable")
				{
					value >> mSerializable;
				}
				else if (value.IsString())
				{
					if		(tag == "Vertex")		_SetSourcePath(value.AsString(), Type::Vertex);
					else if (tag == "Fragment")		_SetSourcePath(value.AsString(), Type::Fragment);
				}
			}
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool GLShader::SerializeTo (TreeNode& root) const
{
	if ( mSerializable && (mVertexInfo.mPath.IsValid() || mFragmentInfo.mPath.IsValid()) )
	{
		TreeNode& node = root.AddChild("Shader", mName);

		if (mVertexInfo.mPath.IsValid())	node.AddChild("Vertex", mVertexInfo.mPath);
		if (mFragmentInfo.mPath.IsValid())  node.AddChild("Fragment", mFragmentInfo.mPath);
	}
	return true;
}