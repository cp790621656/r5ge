#include "../Include/_All.h"
#include "../Include/_OpenGL.h"

// Built-in shaders
#include "../Shaders/ProjectedTexture.h"
#include "../Shaders/Deferred.h"
#include "../Shaders/Lights.h"
#include "../Shaders/SSAO.h"
#include "../Shaders/Blur.h"
#include "../Shaders/Bloom.h"
#include "../Shaders/DOF.h"
#include "../Shaders/Shadow.h"

using namespace R5;

//============================================================================================================
// Common preprocessing function that removes the matched value
//============================================================================================================

uint PreprocessCommon (const String& source, const String& match, Array<String*>& words)
{
	uint length = source.GetLength();
	uint phrase = source.Find(match);

	if (phrase < length)
	{
		String line, vertex, normal, tangent;

		// Extract the entire macroed line
		uint lineEnd = source.GetLine(line, phrase + match.GetLength());

		// Extract the names of the variables
		uint offset = 0;

		FOREACH(i, words)
		{
			offset = line.GetWord(*words[i], offset);
		}
		return lineEnd;
	}
	return length;
}

//============================================================================================================

uint PreprocessCommon (const String& source, const String& match, String& v0)
{
	Array<String*> words;
	words.Expand() = &v0;
	return PreprocessCommon(source, match, words);
}

//============================================================================================================

uint PreprocessCommon (const String& source, const String& match, String& v0, String& v1)
{
	Array<String*> words;
	words.Expand() = &v0;
	words.Expand() = &v1;
	return PreprocessCommon(source, match, words);
}

//============================================================================================================

uint PreprocessCommon (const String& source, const String& match, String& v0, String& v1, String& v2)
{
	Array<String*> words;
	words.Expand() = &v0;
	words.Expand() = &v1;
	words.Expand() = &v2;
	return PreprocessCommon(source, match, words);
}

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
// Preprocess deprecated GLSL vertex shader functionality, replacing such things as 'gl_MultiTexCoord1' with
// their equivalent vertex attribute names.
//============================================================================================================

void PreprocessAttributes (String& source)
{
	String mtc ("gl_MultiTexCoord");
	String match0, match1;

	// ATI has an interesting list of features on their drivers... They don't like gl_MultiTexCoord1 because
	// it has been deprecated in the latest GLSL specs (and I assume all higher gl_MultiTexCoords as well),
	// and need to have them replaced with vertex attributes instead. Fine, but at the same time ATI drivers
	// don't like it when gl_MultiTexCoord0 gets replaced with a vertex attribute as well! Sigh.

	for (uint i = 8; i > 1; )
	{
		match0 = mtc;
		match0 << --i;
		match1 = match0;
		match1 << ".xyz";

		if (source.Contains(match0))
		{
			if (source.Contains(match1))
			{
				source.Replace(match1, String("R5_texCoord%u.xyz", i));
				source = String("attribute vec3 R5_texCoord%u;\n", i) + source;
			}
			else
			{
				source.Replace(match0, String("R5_texCoord%u", i));
				source = String("attribute vec2 R5_texCoord%u;\n", i) + source;
			}
		}
	}
}

//============================================================================================================
// Macro that implements common vertex output functionality including adding forward rendering lighting code
//============================================================================================================
// R5_VERTEX_OUTPUT vertex
//============================================================================================================

bool PreprocessVertexOutput (String& source, bool deferred)
{
	String left, right, vertex, color;

	uint offset = ::PreprocessCommon(source, "R5_VERTEX_OUTPUT", vertex, color);

	if (vertex.IsValid())
	{
		source.GetString(left, 0, offset);
		source.GetString(right, offset);

		left << "\n	gl_Position = gl_ModelViewProjectionMatrix * ";
		left << vertex;

		if (color.IsValid())
		{
			left << ";\n	gl_FrontColor = ";
			left << color;
		}

		left << ";\n";

		if (deferred)
		{
			source = left;
			source << right;
		}
		else
		{
			// Forward rendering needs additional code to calculate per-vertex lighting and fog
			left << "\n{\n";
			left << "	_eyeDir = (gl_ModelViewMatrix * ";
			left << vertex;
			left << ").xyz;\n";
			left << "	_fogFactor = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);\n";

   			left << "	if ( gl_LightSource[0].position.w == 0.0 )\n";
			left << "	{\n";
			left << "		_light.xyz = normalize(gl_LightSource[0].position.xyz);\n";
			left << "		_light.w = 1.0;\n";
			left << "	}\n";
			left << "	else\n";
			left << "	{\n";
			left << "		vec3 eyeToLight = gl_LightSource[0].position.xyz - _eyeDir;\n";
			left << "		float dist = length(eyeToLight);\n";
			left << "		float atten = 1.0 - clamp(dist / gl_LightSource[0].constantAttenuation, 0.0, 1.0);\n";

			left << "		_light.xyz = normalize(eyeToLight);\n";
			left << "		_light.w = pow(atten, gl_LightSource[0].linearAttenuation);\n";
			left << "	}\n";
			left << "}\n";

			// Varyings for eye direction, light properties and fog factor must be defined up top
			source = "varying vec3 _eyeDir;\n";
			source << "varying vec4 _light;\n";
			source << "varying float _fogFactor;\n";
			source << left;
			source << right;
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Macro that implements common fragment shader output functionality used by the engine
//============================================================================================================
// R5_FRAGMENT_OUTPUT diffuse maps normal
// - 'normal' contains the pixel's normal (XYZ) and shininess (W)
// - 'diffuse' is the pixel's diffuse color (RGBA)
// - 'maps' contains specularity (R), specular hue (G), glow (B), and occlusion (A)
//============================================================================================================

bool PreprocessFragmentOutput (String& source, bool deferred, bool shadowed)
{
	String left, right, normal, diffuse, maps;

	uint offset = ::PreprocessCommon(source, "R5_FRAGMENT_OUTPUT", diffuse, maps, normal);

	if (normal.IsValid())
	{
		source.GetString(left, 0, offset);
		source.GetString(right, offset);

		if (deferred)
		{
			// Deferred rendering output is simple -- it's copied nearly as-is
			source = left;
			source << "\n	normal.xyz = normal.xyz * 0.5 + 0.5;";
			source << "\n	gl_FragData[0] = ";
			source << diffuse;
			source << ";\n	gl_FragData[1] = ";
			source << maps;
			source << ";\n	gl_FragData[2] = ";
			source << normal;
			source << ";\n";
			source << right;
		}
		else
		{
			// Forward rendering should add lighting
			left << "\n	vec3 eyeDir   = normalize(_eyeDir);\n";
			left << "	vec3 lightDir = normalize(_light.xyz);\n";

			// Shininess
			left << "	float shininess = ";
			left << normal;
			left << ".w;\n";
			left << "	shininess = 4.0 + shininess * shininess * 250.0;\n";

			// Diffuse factor
			left << "	float diffuseFactor = max(0.0, dot(";
			left << normal;
			left << ".xyz, lightDir));\n";

			// Reflective factor
			left << "	float reflectiveFactor = max(0.0, dot(reflect(lightDir, ";
			left << normal;
			left << ".xyz), eyeDir));\n";

			// Specular factor
			left << "	float specularFactor = pow(reflectiveFactor, shininess);";

			// Taking diffuse into account avoids the "halo" artifact. pow(3) smooths it out.
			//left << "	float invDiff = 1.0 - diffuseFactor;\n";
			//left << "	specularFactor *= 1.0 - invDiff * invDiff * invDiff;\n";

			// Apply the shadow
			if (shadowed)
			{
				uint offset(0), texture(0);
				String temp;

				// Find all existing used textures and figure out the next unused texture's index
				for (;;)
				{
					offset = source.Find("R5_texture", true, offset) + 10;

					if (offset < source.GetLength())
					{
						uint current (0);
						source.GetWord(temp, offset);
						if (temp >> current && texture < current + 1) texture = current + 1;
						++offset;
					}
					else break;
				}

				// Remember whether the shader already used R5_pixelSize or not
				bool pixelSizeExists = source.Find("R5_pixelSize") < source.GetLength();

				// Append the shadowmap texture sampling, mixing it with diffuse and specular factors
				left << "	float shadowFactor = texture2D(R5_texture";
				left << texture;
				left << ", gl_FragCoord.xy * R5_pixelSize).a;\n";
				left << "	diffuseFactor  = min(diffuseFactor, shadowFactor);\n";
				left << "	specularFactor = min(diffuseFactor, specularFactor);\n";

				// Prepend the shadow texture definition
				source = left;
				left = "uniform sampler2D R5_texture";
				left << texture;
				left << ";\n";

				// Prepend the pixel size if it hasn't been used previously
				if (!pixelSizeExists) left << "uniform vec2 R5_pixelSize;\n";

				// Append the rest of the current buffer
				left << source;
			}

			// Material diffuse
			left << "	vec3 matDiff = gl_LightSource[0].diffuse.rgb * ";
			left << diffuse;
			left << ".rgb * diffuseFactor;\n";

			// Material color attenuated by light
			left << "	vec3 color = (gl_LightSource[0].ambient.rgb + matDiff) * _light.w;\n";

			// Material color should transition back to its original unlit color as the 'glow' parameter grows
			left << "	color = mix(color, ";
			left << diffuse;
			left << ".rgb, ";
			left << maps;
			left << ".b);\n";

			// Specular color should be affected by specularity and the specular factor
			left << "	vec3 specular = vec3(";
			left << maps;
			left << ".r * specularFactor);\n";

			// Add specular component to the material color
			left << "	color += mix(specular, specular * ";
			left << diffuse;
			left << ".rgb, ";
			left << maps;
			left << ".g) * _light.w;\n";

			// Make the material color fade out with fog
			left << "	color = mix(color, gl_Fog.color.rgb, _fogFactor);\n";

			// Final color
			left << "	gl_FragColor = vec4(color, diffuse.a);\n";

			// Varyings for eye direction, light properties and fog factor must be defined up top
			source =  "varying vec3 _eyeDir;\n";
			source << "varying float _fogFactor;\n";
			source << "varying vec4 _light;\n";
			source << left;
			source << right;
		}
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
		if (mName.BeginsWith("[R5] Projected Texture"))
		{
			if (mName.Contains("Replace"))
			{
				mCode = g_projectedTexture;
				mCode << g_projectedTextureReplace;
			}
			else if (mName.Contains("Modulate"))
			{
				mCode = g_projectedTexture;
				mCode << g_projectedTextureModulate;
			}
			else
			{
				mCode = g_projectedTexture;
				mCode << g_projectedTextureAddSubtract;
			}
		}
		else if	(mName == "[R5] Combine Deferred")		mCode = g_combineDeferred;
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

			if (ao && shadow)		mCode = g_lightPrefix4;
			else if (ao || shadow)	mCode = g_lightPrefix3;
			else					mCode = g_lightPrefix2;

			mCode << g_lightCommon;

			// Light-specific code
			if (mName.Contains("Directional"))
			{
				mCode << g_lightDirectional;
				mCode << g_lightBody;

				if (ao && shadow)	mCode << g_lightShadowAO;
				else if (ao)		mCode << g_lightAO;
				else if (shadow)	mCode << g_lightShadow;

				mCode << (ao ? g_lightDirAOEnd : g_lightDirEnd);
			}
			else
			{
				mCode << g_lightPoint;
				mCode << g_lightBody;

				if (ao && shadow)	mCode << g_lightShadowAO;
				else if (ao)		mCode << g_lightAO;
				else if (shadow)	mCode << g_lightShadow;

				mCode << (ao ? g_lightPointAOEnd : g_lightPointEnd);
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

				// Shadow cascades need an additional parameter specifying where the shadows start
				if (cascades > 1) mCode << "uniform vec3 shadowStart;\n";

				// Next comes the common component
				mCode << g_shadow;
				mCode << "vec4 pos4 = shadowMatrix0 * worldPos;\n";
				mCode << "float final = SamplePCF(R5_texture1, pos4.xyz / pos4.w);\n";

				// Run through all remaining cascades and add them up
				for (uint i = 1; i < cascades; ++i)
				{
					// Set the new shadow matrix
					mCode << "pos4 = shadowMatrix";
					mCode << i;
					mCode << " * worldPos;\n";

					if (i == 1) mCode << "float ";

					// Sample the shadow texture
					mCode << "shadow = SamplePCF(R5_texture";
					mCode << (i + 1);
					mCode << ", pos4.xyz / pos4.w);\n";

					// Sample the texture and choose the smallest value
					mCode << "final = mix(final, shadow, clamp((depth - shadowStart.";

					// Doing a ceil() on the value here would produce sharp discontinuities between
					// shadows. Doing a division by half a percent here creates a smooth transition.
					if		(i == 1) mCode << "x) * 200.0, 0.0, 1.0));\n";
					else if (i == 2) mCode << "y) * 200.0, 0.0, 1.0));\n";
					else if (i == 3) mCode << "z) * 200.0, 0.0, 1.0));\n";
				}

				// Set the color and end the function
				mCode << "gl_FragColor = vec4(final);\n}";
			}
		}
#ifdef _DEBUG
		else ASSERT(false, "Unrecognized internal shader request");
#endif
	}
	else
	{
		// Encoded parameters, such as: "Shaders/Material [Forward Shadowed]"
		uint index = mName.Find("[");

		if (index < mName.GetSize())
		{
			String temp, source, tags;

			// Source
			mName.GetString(temp, 0, index);
			temp.GetTrimmed(source);

			// Parameters
			mName.GetString(temp, index+1, mName.Find("]", true, index+1, -1, true));
			temp.GetTrimmed(tags);

			// Load the shader
			mCode.Load(source);

			// Pre-process the code
			if (mCode.IsValid()) _Preprocess(tags.Contains("Deferred"), tags.Contains("Shadowed"));
			return;
		}
		else
		{
			// Try to load the code from a file
			mCode.Load(mName);
		}
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

void GLSubShader::_Preprocess (bool deferred, bool shadowed)
{
	mFlags.Clear();
	mDependencies.Clear();

	// Figure out what type of shader this is
	if (mCode.Contains("EndPrimitive();")) mType = Type::Geometry;
	else if (mCode.Contains("gl_Position") || mCode.Contains("R5_VERTEX_OUTPUT")) mType = Type::Vertex;
	else mType = Type::Fragment;

	// Pre-process all macros
	if (mType == Type::Vertex)
	{
		// Pre-processes deprecated GLSL functionality, such as 'gl_MultiTexCoord1',
		// replacing it appropriate vertex attributes.
		::PreprocessAttributes(mCode);

		if (::PreprocessSkinning(mCode))		mFlags.Set(IShader::Flag::Skinned,		true);
		if (::PreprocessInstancing(mCode))		mFlags.Set(IShader::Flag::Instanced,	true);
		if (::PreprocessBillboarding(mCode))	mFlags.Set(IShader::Flag::Billboarded,	true);

		// Vertex shader forward lighting
		::PreprocessVertexOutput(mCode, deferred);
	}
	else
	{
		// Vertex shader forward lighting
		::PreprocessFragmentOutput(mCode, deferred, shadowed);

		bool matShader (false);
		matShader |= mCode.Replace("R5_MATERIAL_SPECULARITY",	"gl_FrontMaterial.specular.r") != 0;
		matShader |= mCode.Replace("R5_MATERIAL_SPECULAR_HUE",	"gl_FrontMaterial.specular.g") != 0;
		matShader |= mCode.Replace("R5_MATERIAL_REFLECTIVENESS","gl_FrontMaterial.specular.b") != 0;
		matShader |= mCode.Replace("R5_MATERIAL_SHININESS",		"gl_FrontMaterial.specular.a") != 0;
		matShader |= mCode.Replace("R5_MATERIAL_OCCLUSION",		"gl_FrontMaterial.emission.r + gl_FrontMaterial.emission.g") != 0;
		matShader |= mCode.Replace("R5_MATERIAL_GLOW",			"gl_FrontMaterial.emission.a") != 0;

		mFlags.Set(IShader::Flag::Material, matShader);
		mFlags.Set(IShader::Flag::Shadowed, shadowed && !deferred);
	}

	// Common flags
	if (mCode.Contains("R5_worldScale")) mFlags.Set(IShader::Flag::WorldScale, true);

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

		Array<String> lines;
		R5::CreateDebugLog(lines, log, mCode);

		if (retVal == GL_TRUE)
		{
			System::Log( "[SHADER]  '%s' has compiled successfully", mName.GetBuffer() );
		}
		else
		{
			System::Log( "[SHADER]  '%s' has FAILED to compile!", mName.GetBuffer() );
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