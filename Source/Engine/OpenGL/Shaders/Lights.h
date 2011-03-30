#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred light shaders
//============================================================================================================

//============================================================================================================
// By default, light shaders only contain 2 textures
//============================================================================================================

static const char* g_lightPrefix2 = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// View space normal and material shininess
};

//============================================================================================================
// Light + shadows
//============================================================================================================

static const char* g_lightPrefix3 = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// View space normal and material shininess
"uniform sampler2D	R5_texture2;\n"		// Shadows
};

//============================================================================================================
// Light + ambient occlusion + shadows
// NOTE: AO takes over channel 2 because it's also the channel potentially used by AO tex for deferred combine
//============================================================================================================

static const char* g_lightPrefix4 = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// View space normal and material shininess
"uniform sampler2D	R5_texture2;\n"		// Ambient Lightmap
"uniform sampler2D	R5_texture3;\n"		// Shadows
};

//============================================================================================================
// All light shaders start the same
//============================================================================================================

static const char* g_lightCommon = {
"uniform mat4 R5_inverseProjMatrix;\n"	// Inverse projection matrix
"uniform vec2 R5_pixelSize;\n"			// (1 / width, 1 / height)

// Gets the view space position at the specified texture coordinates
"vec3 GetViewPos (in vec2 texCoord)\n"
"{\n"
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	vec4 view = vec4(texCoord.x, texCoord.y, depth, 1.0);\n"
"	view.xyz *= 2.0;\n"
"	view.xyz -= 1.0;\n"
"	view = R5_inverseProjMatrix * view;\n"
"	return view.xyz / view.w;\n"
"}\n"

"void main()\n"
"{\n"
	// Figure out the pixel's texture coordinates
"	vec2 texCoord = gl_FragCoord.xy * R5_pixelSize;\n"

	// Get the depth at this pixel
"	vec3 view = GetViewPos(texCoord);\n"
};

//============================================================================================================
// Directional light sources are simple
//============================================================================================================

static const char* g_lightDirectional = {
	// Light direction
"	vec3 light = gl_LightSource[0].position.xyz;\n"
};

//============================================================================================================
// Point light sources are a bit more complicated
//============================================================================================================

static const char* g_lightPoint = {
	// Determine this pixel's distance to the light source
"	vec3 light = gl_LightSource[0].position.xyz - view;\n"
"	float dist = length(light);\n"

	// If the pixel is out of range, discard it
"	float range = gl_LightSource[0].constantAttenuation;\n"
"	if (dist > range) discard;\n"

	// Light's attenuation is stored in the constant attenuation parameter
"	float atten = 1.0 - dist / range;\n"

	// Light's power is stored in linear attenuation parameter
"	atten = pow(atten, gl_LightSource[0].linearAttenuation);\n"
};

//============================================================================================================
// Mid-point for a light shader
//============================================================================================================

static const char* g_lightBody = {
	// Normalize our view and light vectors
"	view  = normalize(view);\n"
"	light = normalize(light);\n"

	// Get the view space normal
"	vec4  normalMap = texture2D(R5_texture1, texCoord);\n"
"	vec3  normal	= normalize(normalMap.xyz * 2.0 - 1.0);\n"

	// Calculate contribution factors (view is flipped, so subtract instead of add here)
"	float shininess 		= 4.0 + normalMap.w * normalMap.w * 250.0;\n"
"	float diffuseFactor 	= max(0.0, dot(normal, light));\n"
"	float reflectiveFactor 	= max(0.0, dot(view, reflect(light, normal)));\n"
"	float specularFactor 	= pow(reflectiveFactor, shininess);\n"
};

//============================================================================================================

static const char* g_lightAO = {
"	float lightmap = texture2D(R5_texture2, texCoord).a;\n"
};

//============================================================================================================

static const char* g_lightShadow = {
"	float shadowFactor 	= texture2D(R5_texture2, gl_FragCoord.xy * R5_pixelSize).a;\n"
"	diffuseFactor  		= min(diffuseFactor, shadowFactor);\n"
"	specularFactor 		= min(diffuseFactor, specularFactor);\n"
};

//============================================================================================================

static const char* g_lightShadowAO = {
"	float lightmap		= texture2D(R5_texture2, texCoord).a;\n"
"	float shadowFactor 	= texture2D(R5_texture3, gl_FragCoord.xy * R5_pixelSize).a;\n"
"	diffuseFactor  		= min(diffuseFactor, shadowFactor);\n"
"	specularFactor 		= min(diffuseFactor, specularFactor);\n"
};

//============================================================================================================
// End of the shader for a directional light
//============================================================================================================

static const char* g_lightDirEnd = {
"	gl_FragData[0] = gl_LightSource[0].ambient + gl_LightSource[0].diffuse * diffuseFactor;\n"
"	gl_FragData[1] = gl_LightSource[0].specular * specularFactor;\n"
"}\n"
};

//============================================================================================================
// End of the shader for a directional light with Ambient Occlusion
//============================================================================================================

static const char* g_lightDirAOEnd = {
"	gl_FragData[0] = gl_LightSource[0].ambient * lightmap + \n"
"					 gl_LightSource[0].diffuse * (diffuseFactor * (0.75 + 0.25 * lightmap));\n"
"	gl_FragData[1] = gl_LightSource[0].specular * specularFactor;\n"
"}\n"
};

//============================================================================================================
// End of the shader for a point light
//============================================================================================================

static const char* g_lightPointEnd = {
"	gl_FragData[0] = gl_LightSource[0].ambient * atten + \n"
"					 gl_LightSource[0].diffuse * (diffuseFactor * atten);\n"
"	gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);\n"
"}\n"
};

//============================================================================================================
// End of the shader for a point light with Ambient Occlusion
//============================================================================================================

static const char* g_lightPointAOEnd = {
"	gl_FragData[0] = gl_LightSource[0].ambient * (atten * lightmap) + \n"
"					 gl_LightSource[0].diffuse * (diffuseFactor * atten * (0.75 + 0.25 * lightmap));\n"
"	gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);\n"
"}\n"
};