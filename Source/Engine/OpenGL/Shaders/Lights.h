#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred light shaders
//============================================================================================================

//============================================================================================================
// By default, light shaders only contain 2 textures
//============================================================================================================

static const char* g_lightPrefix = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// View space normal and material shininess
};

//============================================================================================================
// If ambient occlusion is used, an additional texture is required
//============================================================================================================

static const char* g_lightPrefixAO = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// View space normal and material shininess
"uniform sampler2D	R5_texture2;\n"		// Ambient Lightmap
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
"	view.xyz = view.xyz * 2.0 - 1.0;\n"
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
"	vec3  normal	= normalize(normalMap.rgb * 2.0 - 1.0);\n"

	// Calculate contribution factors (view is flipped, so subtract instead of add here)
"	float shininess 		= 4.0 + (normalMap.a * normalMap.a) * 250.0;\n"
"	float diffuseFactor 	= max(0.0, dot(light, normal));\n"
"	float reflectiveFactor 	= max(0.0, dot(normal, normalize(light - view)));\n"
"	float specularFactor 	= pow(reflectiveFactor, shininess);\n"

   	// Taking diffuse into account avoids the "halo" artifact. pow(3) smooths it out.
"	float temp = 1.0 - diffuseFactor;\n"
"	specularFactor *= 1.0 - temp * temp * temp;\n"
};

//============================================================================================================
// End of the shader for a directional light
//============================================================================================================

static const char* g_lightEndDir = {
"	gl_FragData[0] = gl_LightSource[0].ambient + gl_LightSource[0].diffuse * diffuseFactor;\n"
"	gl_FragData[1] = gl_LightSource[0].specular * specularFactor;\n"
"}\n"
};

//============================================================================================================
// End of the shader for a directional light with Ambient Occlusion
// NOTE: Ideally AO shouldn't affect the diffuse channel. It does here just to make the effect more evident.
//============================================================================================================

static const char* g_lightEndDirAO = {
"	float lightmap = texture2D(R5_texture2, texCoord).r;\n"
"	gl_FragData[0] = (gl_LightSource[0].ambient + gl_LightSource[0].diffuse * diffuseFactor) * lightmap;\n"
"	gl_FragData[1] = gl_LightSource[0].specular * specularFactor;\n"
"}\n"
};

//============================================================================================================
// End of the shader for a point light
//============================================================================================================

static const char* g_lightEndPoint = {
"	gl_FragData[0] = gl_LightSource[0].ambient * atten + gl_LightSource[0].diffuse * (diffuseFactor * atten);\n"
"	gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);\n"
"}\n"
};

//============================================================================================================
// End of the shader for a point light with Ambient Occlusion
// NOTE: Ideally AO shouldn't affect the diffuse channel. It does here just to make the effect more evident.
//============================================================================================================

static const char* g_lightEndPointAO = {
"	float lightmap = texture2D(R5_texture2, texCoord).r;\n"
"	gl_FragData[0] = gl_LightSource[0].ambient * (atten * lightmap) + \n"
"					 gl_LightSource[0].diffuse * (diffuseFactor * atten * lightmap);\n"
"	gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);\n"
"}\n"
};

