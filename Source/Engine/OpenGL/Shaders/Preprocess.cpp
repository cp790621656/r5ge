#include "../Include/_All.h"
#include "Preprocess.h"
using namespace R5;

//============================================================================================================
// R5 shader format needs to be translated to the appropriate API
//============================================================================================================

void ConvertTypes (String& source)
{
	source.Replace("half4", "mediump vec4", true);
	source.Replace("half3", "mediump vec3", true);
	source.Replace("half2", "mediump vec2", true);

	source.Replace("int2", "ivec2", true);
	source.Replace("int3", "ivec3", true);
	source.Replace("int4", "ivec4", true);

	source.Replace("float2", "vec2", true);
	source.Replace("float3", "vec3", true);
	source.Replace("float4", "vec4", true);

	source.Replace("Sample2D(", "texture2D(R5_texture", true);
}

//============================================================================================================
// Adds appropriate R5 functions if the program uses them
//============================================================================================================

void AddReferencedFunctions (String& source)
{
	String prefix;

	if (source.Contains("GetFogFactor", true))
	{
		prefix <<
		"float GetFogFactor (in float depth)\n"
		"{\n"
		"	float linear = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
		"	float fogFactor = clamp((linear - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
		"	return (fogFactor * fogFactor + fogFactor) * 0.5;\n"
		"}\n";
	}

	if (source.Contains("GetLinearDepth", true))
	{
		prefix <<
		"float GetLinearDepth (in float depth)\n"
		"{\n"
		"	return (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
		"}\n";
	}

	source.Replace("GetPixelTexCoords()", "R5_fragCoord.xy * R5_pixelSize", true);
	source.Replace("GetPixelPosition()", "int2(int(R5_fragCoord.x), int(R5_fragCoord.y))", true);

	if (prefix.IsValid()) source = prefix + source;
}

//============================================================================================================
// Add appropriate uniforms
//============================================================================================================

extern Array<GLShader::UniformRecord> g_uniforms;

void AddReferencedUniforms (String& source)
{
	String prefix;

	if (source.Contains("R5_texture0", true)) prefix << "uniform sampler2D R5_texture0;\n";
	if (source.Contains("R5_texture1", true)) prefix << "uniform sampler2D R5_texture1;\n";
	if (source.Contains("R5_texture2", true)) prefix << "uniform sampler2D R5_texture2;\n";
	if (source.Contains("R5_texture3", true)) prefix << "uniform sampler2D R5_texture3;\n";
	if (source.Contains("R5_texture4", true)) prefix << "uniform sampler2D R5_texture4;\n";
	if (source.Contains("R5_texture5", true)) prefix << "uniform sampler2D R5_texture5;\n";
	if (source.Contains("R5_texture6", true)) prefix << "uniform sampler2D R5_texture6;\n";
	if (source.Contains("R5_texture7", true)) prefix << "uniform sampler2D R5_texture7;\n";

	FOREACH(i, g_uniforms)
	{
		const GLShader::UniformRecord& r = g_uniforms[i];

		if (source.Contains(r.name, true))
		{
			if		(r.elements == 16)	prefix << "uniform mat4 ";
			else if (r.elements == 9)	prefix << "uniform mat3 ";
			else if (r.elements == 4)	prefix << "uniform vec4 ";
			else if (r.elements == 3)	prefix << "uniform vec3 ";
			else if (r.elements == 2)	prefix << "uniform vec2 ";
			else						prefix << "uniform float ";

			prefix << r.name;
			prefix << ";\n";
		}
	}
	if (prefix.IsValid()) source = prefix + source;
}

//============================================================================================================
// Common preprocessing function that removes the matched value
//============================================================================================================

uint PreprocessMacroCommon (const String& source, const String& match, Array<String*>& words)
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

uint PreprocessMacroCommon (const String& source, const String& match, String& v0)
{
	Array<String*> words;
	words.Expand() = &v0;
	return PreprocessMacroCommon(source, match, words);
}

//============================================================================================================

uint PreprocessMacroCommon (const String& source, const String& match, String& v0, String& v1)
{
	Array<String*> words;
	words.Expand() = &v0;
	words.Expand() = &v1;
	return PreprocessMacroCommon(source, match, words);
}

//============================================================================================================

uint PreprocessMacroCommon (const String& source, const String& match, String& v0, String& v1, String& v2)
{
	Array<String*> words;
	words.Expand() = &v0;
	words.Expand() = &v1;
	words.Expand() = &v2;
	return PreprocessMacroCommon(source, match, words);
}

//============================================================================================================
// Macro that adds skinning support. Example implementations:
//============================================================================================================
// // R5_IMPLEMENT_SKINNING vertex
// // R5_IMPLEMENT_SKINNING vertex normal
// // R5_IMPLEMENT_SKINNING vertex normal tangent
//============================================================================================================

bool PreprocessMacroSkinning (String& source)
{
	String left, right, vertex, normal, tangent;

	uint offset = ::PreprocessMacroCommon(source, "R5_IMPLEMENT_SKINNING", vertex, normal, tangent);

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

bool PreprocessMacroInstancing (String& source)
{
	String left, right, vertex, normal, tangent;

	uint offset = ::PreprocessMacroCommon(source, "R5_IMPLEMENT_INSTANCING", vertex, normal, tangent);

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

bool PreprocessMacroBillboarding (String& source)
{
	String left, right, vertex, normal, tangent;

	uint offset = ::PreprocessMacroCommon(source, "R5_IMPLEMENT_BILLBOARDING", vertex, normal, tangent);

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

void PreprocessMacroAttributes (String& source)
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

bool PreprocessMacroVertexOutput (String& source, bool deferred)
{
	String left, right, vertex, color;

	uint offset = ::PreprocessMacroCommon(source, "R5_VERTEX_OUTPUT", vertex, color);

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

bool PreprocessMacroFragmentOutput (String& source, bool deferred, bool shadowed)
{
	String left, right, normal, diffuse, maps;

	uint offset = ::PreprocessMacroCommon(source, "R5_FRAGMENT_OUTPUT", diffuse, maps, normal);

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

//============================================================================================================
// Preprocess a new shader format
//============================================================================================================

uint R5::PreprocessShader (String& source, Flags& flags, bool deferred, bool shadowed)
{
	uint type = ISubShader::Type::Invalid;

	if (source.Replace("R5_FRAGMENT_SHADER", "void main()", true))
	{
		String prefix ("#version 130\n");

		::ConvertTypes(source);
		::AddReferencedFunctions(source);
		::AddReferencedUniforms(source);

		if		(source.Contains("R5_finalColor[3]", true)) prefix << "out vec4 R5_finalColor[4];\n";
		else if (source.Contains("R5_finalColor[2]", true)) prefix << "out vec4 R5_finalColor[3];\n";
		else if (source.Contains("R5_finalColor[1]", true)) prefix << "out vec4 R5_finalColor[2];\n";
		else if (source.Replace("R5_finalColor[0]", "R5_finalColor", true)) prefix << "out vec4 R5_finalColor;\n";

		source.Replace("R5_fragCoord", "gl_FragCoord", true);

		source = prefix + source;
		type = ISubShader::Type::Fragment;
	}
	else if (source.Replace("R5_VERTEX_SHADER", "void main()", true))
	{
		type = ISubShader::Type::Vertex;
	}

	// Unknown shader type -- figure it out
	if (type == ISubShader::Type::Invalid)
	{
		if (source.Contains("EndPrimitive();", true))
		{
			type = ISubShader::Type::Geometry;
		}
		else if (source.Contains("gl_Position", true) || source.Contains("R5_VERTEX_OUTPUT", true))
		{
			type = ISubShader::Type::Vertex;
		}
		else
		{
			type = ISubShader::Type::Fragment;
		}
	}

	if (type == ISubShader::Type::Vertex)
	{
		::PreprocessMacroAttributes(source);

		if (::PreprocessMacroSkinning(source))		flags.Set(IShader::Flag::Skinned,		true);
		if (::PreprocessMacroInstancing(source))	flags.Set(IShader::Flag::Instanced,		true);
		if (::PreprocessMacroBillboarding(source))	flags.Set(IShader::Flag::Billboarded,	true);

		// Vertex shader output
		::PreprocessMacroVertexOutput(source, deferred);
	}
	else if (type == ISubShader::Type::Fragment)
	{
		// Raw GLSL fragment shader
		flags.Set(IShader::Flag::Surface, ::PreprocessMacroFragmentOutput(source, deferred, shadowed));

		bool matShader (false);
		matShader |= source.Replace("R5_MATERIAL_SPECULARITY",		"gl_FrontMaterial.specular.r", true) != 0;
		matShader |= source.Replace("R5_MATERIAL_SPECULAR_HUE",		"gl_FrontMaterial.specular.g", true) != 0;
		matShader |= source.Replace("R5_MATERIAL_REFLECTIVENESS",	"gl_FrontMaterial.specular.b", true) != 0;
		matShader |= source.Replace("R5_MATERIAL_SHININESS",		"gl_FrontMaterial.specular.a", true) != 0;
		matShader |= source.Replace("R5_MATERIAL_OCCLUSION",		"gl_FrontMaterial.emission.r + gl_FrontMaterial.emission.g", true) != 0;
		matShader |= source.Replace("R5_MATERIAL_GLOW",				"gl_FrontMaterial.emission.a", true) != 0;

		flags.Set(IShader::Flag::Material, matShader);
		flags.Set(IShader::Flag::Shadowed, shadowed && !deferred);
	}

	// Common macro
	if (source.Contains("R5_worldScale", true)) flags.Set(IShader::Flag::WorldScale, true);
	return type;
}