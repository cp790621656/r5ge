#include "../Include/_All.h"
#include "Preprocess.h"
using namespace R5;

//============================================================================================================
// Adds appropriate surface shader functionality
//============================================================================================================

bool AddSurfaceFunctions (String& source, bool deferred, bool shadowed)
{
	// Remove the last closing bracket
	uint lastBracket = source.Find("}", true, 0, -1, true);
	if (lastBracket >= source.GetSize()) return false;
	source[lastBracket] = '\n';

	// Lighting is on by default for forward rendering shaders.
	// If you need to have a deferred surface that's not affected by light, specify 1 as Material's Glow.
	bool lit = deferred || !source.Contains("#pragma lighting off");

	// Standard Surface Color tag
	if (!source.Contains("R5_surfaceColor", true))
	{
		source << "	R5_surfaceColor = R5_vertexColor * R5_materialColor;\n";
	}

	// Lit shaders need surface and material properties
	if (lit)
	{
		// Standard Surface Properties tag
		if (!source.Contains("R5_surfaceProps", true))
		{
			source << "	R5_surfaceProps = vec4(R5_materialSpecularity, "
				"R5_materialSpecularHue, R5_materialGlow, 1.0);\n";
		}

		// Standard Surface Normal tag
		if (!source.Contains("R5_surfaceNormal", true))
		{
			source << "	R5_surfaceNormal = vec4(normalize(R5_vertexNormal), R5_materialShininess);\n";
		}
	}

	if (deferred)
	{
		// Deferred steps are extremely simple: simply store the values in the 3 output buffers
		source.Replace("R5_surfaceColor", "R5_finalColor[0]", true);
		source.Replace("R5_surfaceProps", "R5_finalColor[1]", true);
		source.Replace("R5_surfaceNormal", "R5_finalColor[2]", true);
	}
	else if (!lit)
	{
		// Non-lit shaders only use the color output
		source.Replace("R5_surfaceColor", "R5_finalColor[0]", true);
		source.Replace("R5_surfaceProps", "//R5_surfaceProps", true);
		source.Replace("R5_surfaceNormal", "//R5_surfaceNormal", true);
	}
	else
	{
		// Lit forward rendering involves lighting calculations
		source.Replace("R5_surfaceColor", "vec4 R5_surfaceColor", true);
		source.Replace("R5_surfaceProps", "vec4 R5_surfaceProps", true);
		source.Replace("R5_surfaceNormal", "vec4 R5_surfaceNormal", true);

		source <<
		"	vec3 eyeDir = normalize(R5_vertexEye);\n"
		"	vec3 lightDir = normalize(R5_vertexLight.xyz);\n"

		"	float diffuseFactor = max(0.0, dot(lightDir, R5_surfaceNormal.xyz));\n"
		"	float reflectiveFactor = max(0.0, dot(reflect(lightDir, R5_surfaceNormal.xyz), eyeDir));\n"
		"	float specularFactor = pow(reflectiveFactor, (R5_surfaceNormal.w * R5_surfaceNormal.w) * 250.0 + 4.0);\n";

		// Add shadows unless they have been purposely turned off
		if (shadowed && !source.Contains("#pragma shadows off", true)) source <<
		"	float shadowFactor = texture2D(R5_shadowMap, GetPixelTexCoords()).a;\n"
		"	diffuseFactor  = min(diffuseFactor, shadowFactor);\n"
		"	specularFactor = min(diffuseFactor, specularFactor);\n";

		source <<
		"	vec3 final = R5_lightDiffuse * (diffuseFactor * (R5_surfaceProps.w * 0.25 + 0.75));\n"
		"	final = (R5_lightAmbient * R5_surfaceProps.w + final) * R5_surfaceColor.rgb * R5_vertexLight.w;\n"
		"	final = mix(final, R5_surfaceColor.rgb, R5_surfaceProps.z);\n"

		"	vec3 specular = vec3(R5_surfaceProps.x * specularFactor);\n"
		"	final += mix(specular, specular * R5_surfaceColor.rgb, R5_surfaceProps.y) * R5_vertexLight.w;\n"
		"	final = mix(final, R5_fogColor.rgb, R5_vertexFog);\n"

		"	R5_finalColor[0] = vec4(final, R5_surfaceColor.a);\n";
	}

	// Restore the final bracket
	source << "}\n";
	return true;
}

//============================================================================================================
// Adds appropriate vertex shader functionality
//============================================================================================================

bool AddVertexFunctions (String& source, bool deferred, Flags& flags)
{
	// Remove the last closing bracket
	uint lastBracket = source.Find("}", true, 0, -1, true);
	if (lastBracket >= source.GetSize()) return false;
	source[lastBracket] = '\n';

	// Vertex position should be a local variable
	source.Replace("R5_vertexPosition", "vec4 R5_vertexPosition", true);

	// TODO: If instanced, use Model * ViewProj
	// Instanced shaders separate model and view matrices in order to allow GPU handle their multiplication
	//bool instanced = source.Contains("#pragma instancing on", true);

	// Skinned shaders allow GPU skinning
	bool skinned = source.Contains("#pragma skinning on", true);
	flags.Set(IShader::Flag::Skinned, skinned);

	// Skinned shaders use an additional set of matrices
	if (skinned)
	{
		source <<
		"	mat4 R5_skinMat = R5_boneTransforms[int(R5_boneIndex.x)] * R5_boneWeight.x +\n"
		"		R5_boneTransforms[int(R5_boneIndex.y)] * R5_boneWeight.y +\n"
		"		R5_boneTransforms[int(R5_boneIndex.z)] * R5_boneWeight.z +\n"
		"		R5_boneTransforms[int(R5_boneIndex.w)] * R5_boneWeight.w;\n"
		"	R5_vertexPosition = R5_skinMat * R5_vertexPosition;\n";
	}

	// Should support instancing
	//if (instanced) source << "	R5_vertexPosition = R5_modelMatrix * R5_vertexPosition;\n";

	// Lit forward rendering needs to calculate the directional vector prior to transforms
	if (!deferred)
	{
		// Direction from the eye position to the vertex in view space
		source << "	R5_vertexEye = (R5_modelViewMatrix * R5_vertexPosition).xyz;\n";

		// Calculate per-vertex fog
		if (!source.Contains("#pragma fog off", true))
		{
			source <<
			"	R5_vertexFog = 1.0 - (R5_clipRange.y + R5_vertexEye.z) / R5_clipRange.w;\n"
			"	R5_vertexFog = clamp((R5_vertexFog - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
			"	R5_vertexFog = 0.5 * (R5_vertexFog * R5_vertexFog + R5_vertexFog);\n";
		}

		if (!source.Contains("#pragma lighting off", true))
		{
			source <<
			"	if (R5_lightPosition.w == 0.0)\n"
			"	{\n"
			"		R5_vertexLight.xyz = normalize(R5_lightPosition.xyz);\n"
			"		R5_vertexLight.w = 1.0;\n"
			"	}\n"
			"	else\n"
			"	{\n"
			"		vec3 eyeToLight = R5_lightPosition.xyz - R5_vertexEye;\n"
			"		float dist = length(eyeToLight);\n"
			"		float atten = 1.0 - clamp(dist / R5_lightParams.x, 0.0, 1.0);\n"
			"		R5_vertexLight.xyz = normalize(eyeToLight);\n"
			"		R5_vertexLight.w = pow(atten, R5_lightParams.y);\n"
			"	}\n";
		}
	}

	// Always include the vertex color
	if (!source.Contains("R5_vertexColor", true)) source << "	R5_vertexColor = R5_color;\n";

	// Transform the normal
	if (source.Contains("R5_vertexNormal", true))
	{
		// Tangent shouldn't exist without the normal
		if (source.Contains("R5_vertexTangent", true))
		{
			source <<
			"	mat3 R5_rotMat;\n";

			if (skinned) source <<
			"	R5_rotMat = mat3(R5_skinMat);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n";
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";

			//if (instanced) source <<
			//"	R5_rotMat = mat3(R5_modelMatrix);\n"
			//"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n";
			//"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";

			source <<
			"	R5_rotMat = mat3(R5_modelViewMatrix);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n";
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";
		}
		else
		{
			if (skinned)	source << "	R5_vertexNormal = mat3(R5_skinMat) * R5_vertexNormal;\n";
			//if (instanced)source << "	R5_vertexNormal = mat3(R5_modelMatrix) * R5_vertexNormal;\n";
							source << "	R5_vertexNormal = mat3(R5_modelViewMatrix) * R5_vertexNormal;\n";
		}
	}

	// Transform the vertex
	source << "	gl_Position = R5_modelViewProjMatrix * R5_vertexPosition;\n";

	// Restore the final bracket
	source << "}\n";
	return true;
}

//============================================================================================================
// Adds appropriate common functions if the program uses them
//============================================================================================================

void AddCommonFunctions (String& source)
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

	if (prefix.IsValid()) source = prefix + source;
}

//============================================================================================================
// Add appropriate uniforms
//============================================================================================================

extern Array<GLShader::UniformRecord> g_uniforms;

void AddReferencedVariables (String& source, bool isFragmentShader)
{
	String prefix;

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

	if (isFragmentShader)
	{
		if (source.Contains("R5_shadowMap", true)) prefix << "uniform sampler2D R5_shadowMap;\n";

		source.Replace("Sample2D(", "texture2D(R5_texture", true);
		source.Replace("SampleCube(", "textureCube(R5_texture", true);

		if (source.Contains("texture2D"))
		{
			const char* header = "uniform sampler2D R5_texture";
			if (source.Contains("texture2D(R5_texture0", true)) { prefix << header; prefix << "0;\n"; }
			if (source.Contains("texture2D(R5_texture1", true)) { prefix << header; prefix << "1;\n"; }
			if (source.Contains("texture2D(R5_texture2", true)) { prefix << header; prefix << "2;\n"; }
			if (source.Contains("texture2D(R5_texture3", true)) { prefix << header; prefix << "3;\n"; }
			if (source.Contains("texture2D(R5_texture4", true)) { prefix << header; prefix << "4;\n"; }
			if (source.Contains("texture2D(R5_texture5", true)) { prefix << header; prefix << "5;\n"; }
			if (source.Contains("texture2D(R5_texture6", true)) { prefix << header; prefix << "6;\n"; }
			if (source.Contains("texture2D(R5_texture7", true)) { prefix << header; prefix << "7;\n"; }
		}

		if (source.Contains("textureCube", true))
		{
			const char* header = "uniform samplerCube ";
			if (source.Contains("textureCube(R5_texture0", true)) { prefix << header; prefix << "0;\n"; }
			if (source.Contains("textureCube(R5_texture1", true)) { prefix << header; prefix << "1;\n"; }
			if (source.Contains("textureCube(R5_texture2", true)) { prefix << header; prefix << "2;\n"; }
			if (source.Contains("textureCube(R5_texture3", true)) { prefix << header; prefix << "3;\n"; }
			if (source.Contains("textureCube(R5_texture4", true)) { prefix << header; prefix << "4;\n"; }
			if (source.Contains("textureCube(R5_texture5", true)) { prefix << header; prefix << "5;\n"; }
			if (source.Contains("textureCube(R5_texture6", true)) { prefix << header; prefix << "6;\n"; }
			if (source.Contains("textureCube(R5_texture7", true)) { prefix << header; prefix << "7;\n"; }
		}
	}
	else
	{
		if (source.Contains("R5_vertex",		 true)) prefix << "in vec4 R5_vertex;\n";
		if (source.Contains("R5_tangent",		 true)) prefix << "in vec4 R5_tangent;\n";
		if (source.Contains("R5_normal",		 true)) prefix << "in vec3 R5_normal;\n";
		if (source.Contains("R5_color",			 true)) prefix << "in vec4 R5_color;\n";
		if (source.Contains("R5_boneWeight",	 true)) prefix << "in vec4 R5_boneWeight;\n";
		if (source.Contains("R5_boneIndex",		 true)) prefix << "in vec4 R5_boneIndex;\n";
		if (source.Contains("R5_texCoord0",		 true)) prefix << "in vec2 R5_texCoord0;\n";
		if (source.Contains("R5_texCoord1",		 true)) prefix << "in vec2 R5_texCoord1;\n";
		if (source.Contains("R5_boneTransforms", true)) prefix << "in mat4 R5_boneTransforms[32];\n";
	}

	const char* inOut = (isFragmentShader ? "in " : "out ");

	if (source.Contains("R5_vertexColor",	true)) { prefix << inOut; prefix << "vec4 R5_vertexColor;\n"; }
	if (source.Contains("R5_vertexEye",		true)) { prefix << inOut; prefix << "vec3 R5_vertexEye;\n"; }
	if (source.Contains("R5_vertexLight",	true)) { prefix << inOut; prefix << "vec4 R5_vertexLight;\n"; }
	if (source.Contains("R5_vertexNormal",	true)) { prefix << inOut; prefix << "vec3 R5_vertexNormal;\n"; }
	if (source.Contains("R5_vertexFog",		true)) { prefix << inOut; prefix << "float R5_vertexFog;\n"; }

	if (prefix.IsValid()) source = prefix + source;
}

//============================================================================================================
// R5 shader format needs to be translated to the appropriate API
//============================================================================================================

void ConvertCommonTypes (String& source)
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
}

//============================================================================================================
// Common preprocessing function that removes the matched value
//============================================================================================================

/*uint PreprocessMacroCommon (const String& source, const String& match, Array<String*>& words)
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
}*/

//============================================================================================================
// Macro that adds skinning support. Example implementations:
//============================================================================================================
// // R5_IMPLEMENT_SKINNING vertex
// // R5_IMPLEMENT_SKINNING vertex normal
// // R5_IMPLEMENT_SKINNING vertex normal tangent
//============================================================================================================

/*bool PreprocessMacroSkinning (String& source)
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
}*/

//============================================================================================================
// Macro that adds pseudo-instancing support. Example implementations:
//============================================================================================================
// // R5_IMPLEMENT_INSTANCING vertex
// // R5_IMPLEMENT_INSTANCING vertex normal
// // R5_IMPLEMENT_INSTANCING vertex normal tangent
//============================================================================================================

/*bool PreprocessMacroInstancing (String& source)
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
}*/

//============================================================================================================
// Macro that adds billboard cloud transform functionality.
//============================================================================================================
// // R5_IMPLEMENT_BILLBOARDING vertex
// // R5_IMPLEMENT_BILLBOARDING vertex normal
// // R5_IMPLEMENT_BILLBOARDING vertex normal tangent
//============================================================================================================

/*bool PreprocessMacroBillboarding (String& source)
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
		left << "offset *= R5_modelScale;\n";
		
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
		source << "uniform float R5_modelScale;\n";
		source << left;
		source << right;
		return true;
	}
	return false;
}*/

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

	if (source.Contains("R5_surface", true))
	{
		flags.Set(IShader::Flag::Surface, true);

		type = ISubShader::Type::Fragment;
		String prefix ("#version 130\n");

		::AddSurfaceFunctions(source, deferred, shadowed);
		::AddCommonFunctions(source);

		// Convenience (fake) uniforms
		source.Replace("R5_materialSpecularity",	"R5_materialParams0.x", true);
		source.Replace("R5_materialSpecularHue",	"R5_materialParams0.y", true);
		source.Replace("R5_materialGlow",			"R5_materialParams0.z", true);
		source.Replace("R5_materialOcclusion",		"R5_materialParams0.w", true);
		source.Replace("R5_materialShininess",		"R5_materialParams1.x", true);
		source.Replace("R5_materialReflectiveness", "R5_materialParams1.y", true);

		// Convenience (fake) functions
		source.Replace("Sample2D(", "texture2D(R5_texture", true);
		source.Replace("GetPixelTexCoords()", "gl_FragCoord.xy * R5_pixelSize", true);
		source.Replace("GetPixelPosition()", "int2(int(gl_FragCoord.x), int(gl_FragCoord.y))", true);
		source.Replace("R5_fragCoord", "gl_FragCoord", true);

		// Fragment shader output values -- support for up to 4 buffers. If you need more, just add them.
		if		(source.Contains("R5_finalColor[3]", true)) prefix << "out vec4 R5_finalColor[4];\n";
		else if (source.Contains("R5_finalColor[2]", true)) prefix << "out vec4 R5_finalColor[3];\n";
		else if (source.Contains("R5_finalColor[1]", true)) prefix << "out vec4 R5_finalColor[2];\n";
		else if (source.Replace("R5_finalColor[0]", "R5_finalColor", true)) prefix << "out vec4 R5_finalColor;\n";

		// Add all referenced variables and convert common types
		::AddReferencedVariables(source, true);
		::ConvertCommonTypes(source);

		// Prepend the prefix
		source = prefix + source;

		//System::Log("=================================================");
		//System::Log(source.GetBuffer());
		//System::Log("=================================================");
	}
	else if (source.Contains("R5_vertexPosition", true))
	{
		type = ISubShader::Type::Vertex;
		String prefix ("#version 130\n");

		::AddVertexFunctions(source, deferred, flags);
		::AddCommonFunctions(source);
		::AddReferencedVariables(source, false);
		::ConvertCommonTypes(source);

		source = prefix + source;
	}
	else
	{
		// This shader uses an outdated format
		flags.Set(IShader::Flag::LegacyFormat, true);
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

	flags.Set(IShader::Flag::Lit, source.Contains("R5_light", true));
	flags.Set(IShader::Flag::Shadowed, shadowed && !deferred);
	return type;
}