#include "../Include/_All.h"
#include "Preprocess.h"
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
// Macro that adds skinning support. Example implementations:
//============================================================================================================
// // R5_IMPLEMENT_SKINNING vertex
// // R5_IMPLEMENT_SKINNING vertex normal
// // R5_IMPLEMENT_SKINNING vertex normal tangent
//============================================================================================================

bool R5::PreprocessSkinning (String& source)
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

bool R5::PreprocessInstancing (String& source)
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

bool R5::PreprocessBillboarding (String& source)
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

void R5::PreprocessAttributes (String& source)
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

bool R5::PreprocessVertexOutput (String& source, bool deferred)
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
			left << "	_fogFactor = 1.0 - (R5_clipRange.y + _eyeDir.z) / R5_clipRange.w;\n";
			left << "	_fogFactor = clamp((_fogFactor - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n";
			left << "	_fogFactor = 0.5 * (_fogFactor + _fogFactor * _fogFactor);\n";

   			left << "	if (gl_LightSource[0].position.w == 0.0)\n";
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
			source  = "uniform vec2 R5_fogRange;\n";
			source << "uniform vec4 R5_clipRange;\n";
			source << "varying vec3 _eyeDir;\n";
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

bool R5::PreprocessFragmentOutput (String& source, bool deferred, bool shadowed)
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
			source << "\n	";
			source << normal;
			source << ".xyz = ";
			source << normal;
			source << ".xyz * 0.5 + 0.5;";
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

			// Material color attenuated by light
			left << "	vec3 color = (gl_LightSource[0].ambient.rgb * ";
			left << maps;
			left << ".a + gl_LightSource[0].diffuse.rgb * (diffuseFactor * (0.75 + 0.25 * ";
			left << maps;
			left << ".a))) *";
			left << diffuse;
			left << ".rgb * _light.w;\n";

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

			// AO visualization:
			//left << "	color = vec3(";
			//left << maps;
			//left << ".a);\n";

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

void R5::PreprocessDependencies (String& source, Array<String>& dependencies)
{
	String match ("R5_INCLUDE ");
	uint offset = 0, length = source.GetLength();
	
	while (length > (offset = source.Find(match, true, offset)))
	{
		offset = source.GetLine(dependencies.Expand(), offset + match.GetLength());
	}
}