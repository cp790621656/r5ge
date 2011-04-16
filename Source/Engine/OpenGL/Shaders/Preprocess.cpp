#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Helper function that cleans up the code code, eliminating extra lines, comments, spaces, etc.
//============================================================================================================

void TrimSource (String& code)
{
	code.Replace("\r\n", "\n", true);
	code.Replace("\t", " ", true);
	while (code.Replace("  ", " ", true)) {}

	String s, line, temp;
	uint offset (0);
	uint ident (0);

	for (;;)
	{
		uint end = code.GetLine(line, offset);

		// Skip comments
		for (uint i = 2; i < line.GetSize(); ++i)
		{
			if (line[i-2] == '/' && line[i-1] == '/')
			{
				line.GetTrimmed(temp, 0, i - 2);
				line = temp;
				break;
			}
		}

		// Empty line? Skip it.
		if (line.GetSize() > 0)
		{
			// Automatic indentation for easier readability
			for (uint i = (line == "}") ? 1 : 0; i < ident; ++i) s << "\t";

			// Calculate indentation adjustments
			for (uint i = 0; i < line.GetSize(); ++i)
			{
				char ch = line[i];
				if (ch == '{') ++ident;
				else if (ch == '}') --ident;
			}

			s << line;
			s << "\n";
		}

		// If we've reached the end, break out
		if (end >= code.GetSize()) break;

		// Adjust the offset and continue
		offset = end;
	}

	//System::Log("=================================================");
	//System::Log(s.GetBuffer());
	//System::Log("=================================================");

	code = s;
}

//============================================================================================================
// Convenience (fake) uniforms
//============================================================================================================

void ProcessMaterials (String& code)
{
	code.Replace("R5_materialSpecularity",		"R5_materialParams0.x", true);
	code.Replace("R5_materialSpecularHue",		"R5_materialParams0.y", true);
	code.Replace("R5_materialGlow",				"R5_materialParams0.z", true);
	code.Replace("R5_materialOcclusion",		"R5_materialParams0.w", true);
	code.Replace("R5_materialShininess",		"R5_materialParams1.x", true);
	code.Replace("R5_materialReflectiveness",	"R5_materialParams1.y", true);
}

//============================================================================================================
// Adds appropriate surface shader functionality
//============================================================================================================

bool ProcessSurfaceShader (String& code, const Flags& desired, Flags& final)
{
	// Standard Surface Color tag
	// Example: R5_surfaceColor = R5_vertexColor * R5_materialColor;
	ASSERT(code.Contains("R5_surfaceColor", true), "Surface shader must always contain R5_surfaceColor output");

	// Shadows and lights are automatically turned off when deferred rendering is requested
	bool deferred = desired.Get(IShader::Flag::Deferred);
	bool shadowed = !deferred && desired.Get(IShader::Flag::Shadowed);
	bool lit = !deferred && desired.Get(IShader::Flag::DirLight | IShader::Flag::PointLight | IShader::Flag::SpotLight);

	// Deferred rendering will be either on or off
	final.Set(IShader::Flag::Deferred, deferred);

	// Remove the last closing bracket
	uint lastBracket = code.Find("}", true, 0, -1, true);
	if (lastBracket >= code.GetSize()) return false;
	code[lastBracket] = '\n';

	// Lit shaders need surface and material properties
	if (lit || deferred)
	{
		// Standard Surface Properties tag
		if (!code.Contains("R5_surfaceProps", true))
		{
			code << "	R5_surfaceProps = vec4(R5_materialSpecularity, "
				"R5_materialSpecularHue, R5_materialGlow, 1.0);\n";
		}

		// Standard Surface Normal tag
		if (!code.Contains("R5_surfaceNormal", true))
		{
			code << "	R5_surfaceNormal = vec4(normalize(R5_vertexNormal), R5_materialShininess);\n";
		}
	}

	if (deferred)
	{
		// Deferred steps are extremely simple: simply store the values in the 3 output buffers
		code.Replace("R5_surfaceColor", "R5_finalColor[0]", true);
		code.Replace("R5_surfaceProps", "R5_finalColor[1]", true);
		code.Replace("R5_surfaceNormal", "vec4 R5_surfaceNormal", true);
		code <<
		"R5_surfaceNormal.xyz = R5_surfaceNormal.xyz * 0.5 + 0.5;\n"
		"R5_finalColor[2] = R5_surfaceNormal;\n";
	}
	else if (!lit)
	{
		// Non-lit shaders only use the color output
		code.Replace("R5_surfaceColor", "R5_finalColor[0]", true);
		code.Replace("R5_surfaceProps", "//R5_surfaceProps", true);
		code.Replace("R5_surfaceNormal", "//R5_surfaceNormal", true);
	}
	else // Lit forward rendering shader
	{
		// Lit forward rendering involves lighting calculations
		code.Replace("R5_surfaceColor", "vec4 R5_surfaceColor", true);
		code.Replace("R5_surfaceProps", "vec4 R5_surfaceProps", true);
		code.Replace("R5_surfaceNormal", "vec4 R5_surfaceNormal", true);

		code << "	vec3 eyeDir = normalize(R5_vertexEye);\n";

		if (desired.Get(IShader::Flag::DirLight))
		{
			final.Set(IShader::Flag::DirLight, true);

			// Directional light code is simple
			code <<
			"	vec3 lightDir = normalize(R5_lightDirection);\n";
		}
		else
		{
			final.Set( desired.Get(IShader::Flag::PointLight) ?
				IShader::Flag::PointLight : IShader::Flag::SpotLight, true);

			// Point lights and spot lights need to take attenuation into account
			code <<
			"	vec3 eyeToLight = R5_lightPosition - R5_vertexEye;\n"
			"	float dist = length(eyeToLight);\n"
			"	float atten = 1.0 - clamp(dist / R5_lightParams.x, 0.0, 1.0);\n"
			"	vec3 lightDir = normalize(eyeToLight);\n"
			"	atten = pow(atten, R5_lightParams.y);\n";
		}

		code <<
		"	float diffuseFactor = max(0.0, dot(lightDir, R5_surfaceNormal.xyz));\n"
		"	float reflectiveFactor = max(0.0, dot(reflect(lightDir, R5_surfaceNormal.xyz), eyeDir));\n"
		"	float specularFactor = pow(reflectiveFactor, (R5_surfaceNormal.w * R5_surfaceNormal.w) * 250.0 + 4.0);\n";

		if (shadowed)
		{
			final.Set(IShader::Flag::Shadowed, true);
			code <<
			"	float shadowFactor = texture2D(R5_shadowMap, GetPixelTexCoords()).a;\n"
			"	diffuseFactor  = min(diffuseFactor, shadowFactor);\n"
			"	specularFactor = min(diffuseFactor, specularFactor);\n";
		}

		code <<
		"	vec3 final = R5_lightDiffuse * (diffuseFactor * (R5_surfaceProps.w * 0.25 + 0.75));\n"
		"	final = (R5_lightAmbient * R5_surfaceProps.w + final) * R5_surfaceColor.rgb";

		code << (desired.Get(IShader::Flag::DirLight) ? ";\n" : " * atten;\n");

		code <<
		"	final = mix(final, R5_surfaceColor.rgb, R5_surfaceProps.z);\n"
		"	vec3 specular = vec3(R5_surfaceProps.x * specularFactor);\n"
		"	final += mix(specular, specular * R5_surfaceColor.rgb, R5_surfaceProps.y)";

		code << (desired.Get(IShader::Flag::DirLight) ? ";\n" : " * atten;\n");
		code <<
		"	final = mix(final, R5_fogColor.rgb, R5_vertexFog);\n"
		"	R5_finalColor[0] = vec4(final, R5_surfaceColor.a);\n";
	}

	// Restore the final bracket
	code << "}\n";
	return true;
}

//============================================================================================================
// Adds appropriate vertex shader functionality
//============================================================================================================

bool AddVertexFunctions (String& code, const Flags& desired, Flags& final)
{
	// Remove the last closing bracket
	uint lastBracket = code.Find("}", true, 0, -1, true);
	if (lastBracket >= code.GetSize()) return false;
	code[lastBracket] = '\n';

	// Vertex position should be a local variable
	code.Replace("R5_vertexPosition", "vec3 R5_vertexPosition", true);

	// Vertex position we'll be working with is a vec4
	code << "	vec4 R5_position = vec4(R5_vertexPosition, 1.0);\n";

	// Skinned shaders use an additional set of matrices
	if (desired.Get(IShader::Flag::Skinned))
	{
		final.Set(IShader::Flag::Skinned, true);

		code <<
		"	mat4 R5_skinMat = R5_boneTransforms[int(R5_boneIndex.x)] * R5_boneWeight.x +\n"
		"		R5_boneTransforms[int(R5_boneIndex.y)] * R5_boneWeight.y +\n"
		"		R5_boneTransforms[int(R5_boneIndex.z)] * R5_boneWeight.z +\n"
		"		R5_boneTransforms[int(R5_boneIndex.w)] * R5_boneWeight.w;\n"
		"	R5_position = R5_skinMat * R5_position;\n";
	}

	// Calculate the vertex position
	code <<
	"	R5_position = R5_modelMatrix * R5_position;\n"
	"	R5_position = R5_viewMatrix * R5_position;\n";

	if (desired.Get(IShader::Flag::Billboard))
	{
		final.Set(IShader::Flag::Billboard, true);

		// Billboard calculations are done in view space
		code <<
		"vec3 R5_offset = R5_texCoord0.xyz;\n"
		"R5_offset.xy = (R5_offset.xy * 2.0 - 1.0) * R5_offset.z;\n"
		"R5_offset.z *= 0.25;\n"
		"R5_position.xyz = R5_offset * R5_modelScale + R5_position.xyz;\n"
		"R5_vertexNormal = R5_vertexPosition.xyz - R5_origin.xyz;\n"
		"R5_vertexTangent = vec3(R5_vertexNormal.y, -R5_vertexNormal.z, 0.0);\n";
	}

	code << "	gl_Position = R5_projMatrix * R5_position;\n";

	uint light = IShader::Flag::DirLight | IShader::Flag::PointLight | IShader::Flag::SpotLight;

	if (!desired.Get(IShader::Flag::Deferred) && desired.Get(light))
	{
		code << "	R5_vertexEye = R5_position.xyz;\n";
		final.Set(desired.Get() & light);

		// Calculate per-vertex fog
		if (desired.Get(IShader::Flag::Fog))
		{
			final.Set(IShader::Flag::Fog, true);
			code <<
			"	R5_vertexFog = 1.0 - (R5_clipRange.y + R5_vertexEye.z) / R5_clipRange.w;\n"
			"	R5_vertexFog = clamp((R5_vertexFog - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
			"	R5_vertexFog = 0.5 * (R5_vertexFog * R5_vertexFog + R5_vertexFog);\n";
		}
	}

	// Always include the vertex color
	if (!code.Contains("R5_vertexColor", true)) code << "	R5_vertexColor = R5_color;\n";

	// Transform the normal
	if (code.Contains("R5_vertexNormal", true))
	{
		// Tangent shouldn't exist without the normal
		if (code.Contains("R5_vertexTangent", true))
		{
			code <<
			"	mat3 R5_rotMat;\n";

			if (desired.Get(IShader::Flag::Skinned)) code <<
			"	R5_rotMat = mat3(R5_skinMat);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n"
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";

			code <<
			"	R5_rotMat = mat3(R5_modelMatrix);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n"
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";

			code <<
			"	R5_rotMat = mat3(R5_viewMatrix);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n"
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";
		}
		else
		{
			if (desired.Get(IShader::Flag::Skinned))
			code << "	R5_vertexNormal = mat3(R5_skinMat) * R5_vertexNormal;\n";
			code << "	R5_vertexNormal = mat3(R5_modelMatrix) * R5_vertexNormal;\n"
					  "	R5_vertexNormal = mat3(R5_viewMatrix) * R5_vertexNormal;\n";
		}
	}

	// Restore the final bracket
	code << "}\n";
	return true;
}

//============================================================================================================
// Helper function that attempts to fix legacy shaders (pre-OpenGL 3.0)
//============================================================================================================

void FixLegacyShader (String& code)
{
	code.Replace("#version 110", "", true);

	// Remove the last closing bracket
	uint lastBracket = code.Find("}", true, 0, -1, true);
	if (lastBracket >= code.GetSize()) return;
	code[lastBracket] = '\n';

	uint lightPos = code.Replace("gl_LightSource[0].position", "R5_lightPosition", true);
	uint ambient =	code.Replace("gl_LightSource[0].ambient", "R5_lightAmbient", true);
	uint diffuse =	code.Replace("gl_LightSource[0].diffuse", "R5_lightDiffuse", true) |
					code.Replace("gl_LightSource[0].specular", "R5_lightDiffuse", true);

	// Previous generation surface shader support
	//uint material = code.Replace("R5_MATERIAL_SPECULARITY", "R5_materialSpecularity", true) |
	//				code.Replace("R5_MATERIAL_SPECULAR_HUE", "R5_materialSpecularHue", true) |
	//				code.Replace("R5_MATERIAL_GLOW", "R5_materialGlow", true) |
	//				code.Replace("R5_MATERIAL_REFLECTIVENESS", "R5_materialReflectiveness", true) |
	//				code.Replace("R5_MATERIAL_SHININESS", "R5_materialShininess", true) |
	//				code.Replace("R5_MATERIAL_OCCLUSION", "R5_materialOcclusion", true) |
	//				code.Replace("gl_FrontLightProduct[0].ambient", "R5_materialColor", true) |
	//				code.Replace("gl_FrontLightProduct[0].diffuse", "R5_materialColor", true) |
	//				code.Replace("gl_FrontLightProduct[0].specular", "R5_materialColor", true);

	uint texCoord0 = code.Replace("gl_MultiTexCoord0", "R5_texCoord0", true);
	uint texCoord1 = code.Replace("gl_MultiTexCoord1", "R5_texCoord1", true);

	String prefix;

	if (texCoord0)	prefix << "attribute vec2 R5_texCoord0;\n";
	if (texCoord1)	prefix << "attribute vec2 R5_texCoord1;\n";
	if (lightPos)	prefix << "uniform vec3 R5_lightPosition;\n";
	if (ambient)	prefix << "uniform vec3 R5_lightAmbient;\n";
	if (diffuse)	prefix << "uniform vec3 R5_lightDiffuse;\n";
	//if (material)	prefix << "uniform vec4 R5_materialColor;\n"
	//						  "uniform vec4 R5_materialParams0;\n"
	//						  "uniform vec2 R5_materialParams1;\n";

	if (prefix.IsValid()) code = prefix + code;

	::ProcessMaterials(code);

	// Restore the final bracket
	code << "}\n";
}

//============================================================================================================
// Adds appropriate common functions if the program uses them
//============================================================================================================

void AddCommonFunctions (String& code)
{
	String prefix;

	if (code.Contains("GetFogFactor", true))
	{
		prefix <<
		"float GetFogFactor (in float depth)\n"
		"{\n"
		"	float linear = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
		"	float fogFactor = clamp((linear - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
		"	return (fogFactor * fogFactor + fogFactor) * 0.5;\n"
		"}\n";
	}

	if (code.Contains("GetLinearDepth", true))
	{
		prefix <<
		"float GetLinearDepth (in float depth)\n"
		"{\n"
		"	return (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
		"}\n";
	}

	if (code.Contains("GetViewPosition", true))
	{
		prefix <<
		"vec3 GetViewPosition (in vec2 texCoord, in float depth)\n"
		"{\n"
		"	vec4 view = vec4(texCoord.x, texCoord.y, depth, 1.0);\n"
		"	view.xyz = view.xyz * 2.0 - 1.0;\n"
		"	view = R5_inverseProjMatrix * view;\n"
		"	return view.xyz / view.w;\n"
		"}\n";
	}

	if (prefix.IsValid()) code = prefix + code;
}

//============================================================================================================
// Add appropriate uniforms
//============================================================================================================

extern Array<GLShaderProgram::UniformRecord> g_uniforms;

void AddReferencedVariables (String& code, bool isFragmentShader)
{
	String prefix;

	FOREACH(i, g_uniforms)
	{
		const GLShaderProgram::UniformRecord& r = g_uniforms[i];

		if (code.Contains(r.name, true))
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
		if (code.Contains("R5_shadowMap", true)) prefix << "uniform sampler2D R5_shadowMap;\n";

		code.Replace("Sample2D(", "texture2D(R5_texture", true);
		code.Replace("SampleCube(", "textureCube(R5_texture", true);

		if (code.Contains("texture2D"))
		{
			const char* header = "uniform sampler2D R5_texture";
			if (code.Contains("texture2D(R5_texture0", true)) { prefix << header; prefix << "0;\n"; }
			if (code.Contains("texture2D(R5_texture1", true)) { prefix << header; prefix << "1;\n"; }
			if (code.Contains("texture2D(R5_texture2", true)) { prefix << header; prefix << "2;\n"; }
			if (code.Contains("texture2D(R5_texture3", true)) { prefix << header; prefix << "3;\n"; }
			if (code.Contains("texture2D(R5_texture4", true)) { prefix << header; prefix << "4;\n"; }
			if (code.Contains("texture2D(R5_texture5", true)) { prefix << header; prefix << "5;\n"; }
			if (code.Contains("texture2D(R5_texture6", true)) { prefix << header; prefix << "6;\n"; }
			if (code.Contains("texture2D(R5_texture7", true)) { prefix << header; prefix << "7;\n"; }
		}

		if (code.Contains("textureCube", true))
		{
			const char* header = "uniform samplerCube ";
			if (code.Contains("textureCube(R5_texture0", true)) { prefix << header; prefix << "0;\n"; }
			if (code.Contains("textureCube(R5_texture1", true)) { prefix << header; prefix << "1;\n"; }
			if (code.Contains("textureCube(R5_texture2", true)) { prefix << header; prefix << "2;\n"; }
			if (code.Contains("textureCube(R5_texture3", true)) { prefix << header; prefix << "3;\n"; }
			if (code.Contains("textureCube(R5_texture4", true)) { prefix << header; prefix << "4;\n"; }
			if (code.Contains("textureCube(R5_texture5", true)) { prefix << header; prefix << "5;\n"; }
			if (code.Contains("textureCube(R5_texture6", true)) { prefix << header; prefix << "6;\n"; }
			if (code.Contains("textureCube(R5_texture7", true)) { prefix << header; prefix << "7;\n"; }
		}
	}
	else
	{
		if (code.Contains("R5_vertex",			true)) prefix << "in vec3 R5_vertex;\n";
		if (code.Contains("R5_tangent",			true)) prefix << "in vec3 R5_tangent;\n";
		if (code.Contains("R5_normal",			true)) prefix << "in vec3 R5_normal;\n";
		if (code.Contains("R5_color",			true)) prefix << "in vec4 R5_color;\n";
		if (code.Contains("R5_boneWeight",		true)) prefix << "in vec4 R5_boneWeight;\n";
		if (code.Contains("R5_boneIndex",		true)) prefix << "in vec4 R5_boneIndex;\n";
		if (code.Contains("R5_texCoord0",		true)) prefix << "in vec2 R5_texCoord0;\n";
		if (code.Contains("R5_texCoord1",		true)) prefix << "in vec2 R5_texCoord1;\n";
		if (code.Contains("R5_boneTransforms",	true)) prefix << "in mat4 R5_boneTransforms[32];\n";
	}

	const char* inOut = (isFragmentShader ? "in " : "out ");

	if (code.Contains("R5_vertexColor",		true)) { prefix << inOut; prefix << "vec4 R5_vertexColor;\n"; }
	if (code.Contains("R5_vertexEye",		true)) { prefix << inOut; prefix << "vec3 R5_vertexEye;\n"; }
	if (code.Contains("R5_vertexLight",		true)) { prefix << inOut; prefix << "vec4 R5_vertexLight;\n"; }
	if (code.Contains("R5_vertexNormal",	true)) { prefix << inOut; prefix << "vec3 R5_vertexNormal;\n"; }
	if (code.Contains("R5_vertexTangent",	true)) { prefix << inOut; prefix << "vec3 R5_vertexTangent;\n"; }
	if (code.Contains("R5_vertexTexCoord0",	true)) { prefix << inOut; prefix << "vec2 R5_vertexTexCoord0;\n"; }
	if (code.Contains("R5_vertexTexCoord1",	true)) { prefix << inOut; prefix << "vec2 R5_vertexTexCoord1;\n"; }
	if (code.Contains("R5_vertexFog",		true)) { prefix << inOut; prefix << "float R5_vertexFog;\n"; }

	if (prefix.IsValid()) code = prefix + code;
}

//============================================================================================================
// R5 shader format needs to be translated to the appropriate API
//============================================================================================================

void ConvertCommonTypes (String& code)
{
	code.Replace("half4", "mediump vec4", true);
	code.Replace("half3", "mediump vec3", true);
	code.Replace("half2", "mediump vec2", true);

	code.Replace("int2", "ivec2", true);
	code.Replace("int3", "ivec3", true);
	code.Replace("int4", "ivec4", true);

	code.Replace("float2", "vec2", true);
	code.Replace("float3", "vec3", true);
	code.Replace("float4", "vec4", true);
}

//============================================================================================================
// Preprocess a new shader format
//============================================================================================================

uint GLPreprocessShader (String& code, const Flags& desired, Flags& final)
{
	bool surface = code.Contains("R5_surface", true);

	if (surface || code.Contains("R5_finalColor", true))
	{
		final.Set(IShader::Flag::Fragment, true);

		if (surface)
		{
			final.Set(IShader::Flag::Surface, true);
			::ProcessSurfaceShader(code, desired, final);
		}

		::AddCommonFunctions(code);
		::ProcessMaterials(code);

		// Convenience (fake) functions
		code.Replace("Sample2D(", "texture2D(R5_texture", true);
		code.Replace("GetPixelTexCoords()", "gl_FragCoord.xy * R5_pixelSize", true);
		code.Replace("GetPixelPosition()", "int2(int(gl_FragCoord.x), int(gl_FragCoord.y))", true);
		code.Replace("R5_fragCoord", "gl_FragCoord", true);

		String prefix ("#version 130\n");

		// Fragment shader output values -- support for up to 4 buffers. If you need more, just add them.
		if		(code.Contains("R5_finalColor[3]", true)) prefix << "out vec4 R5_finalColor[4];\n";
		else if (code.Contains("R5_finalColor[2]", true)) prefix << "out vec4 R5_finalColor[3];\n";
		else if (code.Contains("R5_finalColor[1]", true)) prefix << "out vec4 R5_finalColor[2];\n";
		else if (code.Replace("R5_finalColor[0]", "R5_finalColor", true)) prefix << "out vec4 R5_finalColor;\n";

		// Add all referenced variables and convert common types
		::AddReferencedVariables(code, true);
		::ConvertCommonTypes(code);

		// Prepend the prefix
		code = prefix + code;
		return IShader::Type::Fragment;
	}
	else if (code.Contains("R5_vertexPosition", true))
	{
		final.Set(IShader::Flag::Vertex, true);

		::AddVertexFunctions(code, desired, final);
		::AddCommonFunctions(code);
		::AddReferencedVariables(code, false);
		::ConvertCommonTypes(code);

		code = "#version 130\n" + code;
		return IShader::Type::Vertex;
	}
	else
	{
		// Lights are no longer supported
		if (code.Contains("gl_Light", true) || code.Contains("gl_FrontLight", true))
		{
			WARNING("Legacy shader format, please upgrade");
		}

		// This shader uses an outdated format
		final.Set(IShader::Flag::LegacyFormat, true);

		::TrimSource(code);
		::FixLegacyShader(code);

		if (code.Contains("gl_Position"))
		{
			final.Set(IShader::Flag::Vertex);
			return IShader::Type::Vertex;
		}
		else
		{
			final.Set(IShader::Flag::Fragment);
			return IShader::Type::Fragment;
		}
	}
}
