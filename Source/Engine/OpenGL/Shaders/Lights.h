#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred light shaders
//============================================================================================================

//============================================================================================================
// All light shaders start the same
//============================================================================================================

static const char* g_lightCommon = {
"void main()\n"
"{\n"
	// Figure out the pixel's texture coordinates
"	vec2 texCoord = GetPixelTexCoords();\n"

	// Get the view position of this pixel
"	float depth = Sample2D(0, texCoord).r;\n"
"	vec3 view = GetViewPosition(texCoord, depth);\n"
};

//============================================================================================================
// Directional light sources are simple
//============================================================================================================

static const char* g_lightDirectional = {
	// Light direction
"	vec3 light = R5_lightPosition.xyz;\n"
};

//============================================================================================================
// Point light sources are a bit more complicated
//============================================================================================================

static const char* g_lightPoint = {
	// Determine this pixel's distance to the light source
"	vec3 light = R5_lightPosition.xyz - view;\n"
"	float dist = length(light);\n"

	// If the pixel is out of range, discard it
"	if (dist > R5_lightParams.x) discard;\n"

	// Light's attenuation is stored in the constant attenuation parameter
"	float atten = 1.0 - dist / R5_lightParams.x;\n"

	// Light's power is stored in linear attenuation parameter
"	atten = pow(atten, R5_lightParams.y);\n"
};

//============================================================================================================
// Mid-point for a light shader
//============================================================================================================

static const char* g_lightBody = {
	// Normalize our view and light vectors
"	view  = normalize(view);\n"
"	light = normalize(light);\n"

	// Get the view space normal
"	vec4  normalMap = Sample2D(1, texCoord);\n"
"	vec3  normal	= normalize(normalMap.xyz * 2.0 - 1.0);\n"

	// Calculate contribution factors (view is flipped, so subtract instead of add here)
"	float shininess 		= (normalMap.w * normalMap.w) * 250.0 + 4.0;\n"
"	float diffuseFactor 	= max(0.0, dot(normal, light));\n"
"	float reflectiveFactor 	= max(0.0, dot(view, reflect(light, normal)));\n"
"	float specularFactor 	= pow(reflectiveFactor, shininess);\n"
};

//============================================================================================================

static const char* g_lightAO = {
"	float lightmap = Sample2D(2, texCoord).a;\n"
};

//============================================================================================================

static const char* g_lightShadow = {
"	float shadowFactor 	= Sample2D(2, gl_FragCoord.xy * R5_pixelSize).a;\n"
"	diffuseFactor  		= min(diffuseFactor, shadowFactor);\n"
"	specularFactor 		= min(diffuseFactor, specularFactor);\n"
};

//============================================================================================================

static const char* g_lightShadowAO = {
"	float lightmap		= Sample2D(2, texCoord).a;\n"
"	float shadowFactor 	= Sample2D(3, gl_FragCoord.xy * R5_pixelSize).a;\n"
"	diffuseFactor  		= min(diffuseFactor, shadowFactor);\n"
"	specularFactor 		= min(diffuseFactor, specularFactor);\n"
};

//============================================================================================================
// End of the shader for a directional light
//============================================================================================================

static const char* g_lightDirEnd = {
"	R5_finalColor[0] = vec4(R5_lightDiffuse * diffuseFactor + R5_lightAmbient, 1.0);\n"
"	R5_finalColor[1] = vec4(R5_lightDiffuse * specularFactor, 1.0);\n"
"}\n"
};

//============================================================================================================
// End of the shader for a directional light with Ambient Occlusion
//============================================================================================================

static const char* g_lightDirAOEnd = {
"	vec3 diff = R5_lightDiffuse * (diffuseFactor * (lightmap * 0.25 + 0.75));\n"
"	R5_finalColor[0] = vec4(R5_lightAmbient * lightmap + diff, 1.0);\n"
"	R5_finalColor[1] = vec4(R5_lightDiffuse * specularFactor, 1.0);\n"
"}\n"
};

//============================================================================================================
// End of the shader for a point light
//============================================================================================================

static const char* g_lightPointEnd = {
"	R5_finalColor[0] = vec4(R5_lightAmbient * atten + (R5_lightDiffuse * (diffuseFactor * atten)), 1.0);\n"
"	R5_finalColor[1] = vec4(R5_lightDiffuse * (specularFactor * atten), 1.0);\n"
"}\n"
};

//============================================================================================================
// End of the shader for a point light with Ambient Occlusion
//============================================================================================================

static const char* g_lightPointAOEnd = {
"	diffuseFactor *= atten;\n"
"	specularFactor *= atten;\n"
"	vec3 diff = R5_lightDiffuse * (diffuseFactor * (lightmap * 0.25 + 0.75));\n"
"	R5_finalColor[0] = vec4(R5_lightAmbient * (atten * lightmap) + diff, 1.0);\n"
"	R5_finalColor[1] = vec4(R5_lightDiffuse * specularFactor, 1.0);\n"
"}\n"
};