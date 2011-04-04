#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred lighting final combination shader
//============================================================================================================

/*static const char* g_combineDeferredF = {
"#version 150\n"
"uniform sampler2DMS	R5_texture0;\n"		// Depth
"uniform sampler2DMS	R5_texture1;\n"		// Material Diffuse (RGBA)
"uniform sampler2DMS	R5_texture2;\n"		// Material Specular Intensity (R), Specular Hue (G), Self-Illumination (B), Ambient Occlusion (A)
"uniform sampler2DMS	R5_texture3;\n"		// Light Diffuse (RGB)
"uniform sampler2DMS	R5_texture4;\n"		// Light Specular (RGB)

"uniform vec4		R5_clipRange;\n"
"uniform vec2		R5_fogRange;\n"		// X = 0-1 fog's start, Y = 0-1 fog's range.
"uniform vec4		R5_fogColor;\n"

"out vec4 FinalColor;\n"

"void main()\n"
"{\n"
"	ivec2 itc = ivec2(int(gl_FragCoord.x * 0.5), int(gl_FragCoord.y));\n"
"	float depth		= texelFetch(R5_texture0, itc, 0).r;\n"
"	vec4 matDiff    = texelFetch(R5_texture1, itc, 0);\n"
"	vec4 matMaps    = texelFetch(R5_texture2, itc, 0);\n"
"	vec3 lightDiff  = texelFetch(R5_texture3, itc, 0).rgb;\n"
"	vec3 lightSpec  = texelFetch(R5_texture4, itc, 0).rgb;\n"
"	vec3 specular	= lightSpec * matMaps.r;\n"
"	vec3 color		= mix(lightDiff * matDiff.rgb, matDiff.rgb, matMaps.b) +\n"
"					  mix(specular, specular * matDiff.rgb, matMaps.g);\n"

// AO visualization:
//"	color = vec3(matMaps.a, matMaps.a, matMaps.a);\n"

	// Make the depth linear
"	depth = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"

	// Fog contribution
"	float fogFactor = clamp((depth - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
"	fogFactor = 0.5 * (fogFactor + fogFactor * fogFactor);\n"
"	FinalColor = vec4( mix(color, R5_fogColor.rgb, fogFactor), matDiff.a );\n"
"}\n"};*/

static const char* g_combineDeferred = {
"#version 150\n"

"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// Material Diffuse (RGBA)
"uniform sampler2D	R5_texture2;\n"		// Material Specular Intensity (R), Specular Hue (G), Self-Illumination (B), Ambient Occlusion (A)
"uniform sampler2D	R5_texture3;\n"		// Light Diffuse (RGB)
"uniform sampler2D	R5_texture4;\n"		// Light Specular (RGB)

"uniform vec4		R5_clipRange;\n"
"uniform vec2		R5_fogRange;\n"		// X = 0-1 fog's start, Y = 0-1 fog's range.
"uniform vec4		R5_fogColor;\n"

"out vec4 FinalColor;\n"

"void main()\n"
"{\n"
"	ivec2 tc = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));\n"

"	float depth		= texelFetch(R5_texture0, tc, 0).r;\n"
"	vec4 matDiff    = texelFetch(R5_texture1, tc, 0);\n"
"	vec4 matMaps    = texelFetch(R5_texture2, tc, 0);\n"
"	vec3 lightDiff  = texelFetch(R5_texture3, tc, 0).rgb;\n"
"	vec3 lightSpec  = texelFetch(R5_texture4, tc, 0).rgb;\n"

"	vec3 specular	= lightSpec * matMaps.r;\n"
"	vec3 color		= mix(lightDiff * matDiff.rgb, matDiff.rgb, matMaps.b) +\n"
"					  mix(specular, specular * matDiff.rgb, matMaps.g);\n"

// AO visualization:
//"	color = vec3(matMaps.a, matMaps.a, matMaps.a);\n"

	// Make the depth linear
"	depth = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"

	// Fog contribution
"	float fogFactor = clamp((depth - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
"	fogFactor = (fogFactor * fogFactor + fogFactor) * 0.5;\n"
"	FinalColor = vec4( mix(color, R5_fogColor.rgb, fogFactor), matDiff.a );\n"
"}\n"};