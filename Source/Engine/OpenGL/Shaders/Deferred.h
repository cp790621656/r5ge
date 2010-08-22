#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred lighting final combination shader
//============================================================================================================

static const char* g_combineDeferred = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// Material Diffuse (RGBA)
"uniform sampler2D	R5_texture2;\n"		// Material Specular Intensity (R), Specular Hue (G), Self-Illumination (B), Ambient Occlusion (A)
"uniform sampler2D	R5_texture3;\n"		// Light Diffuse (RGB)
"uniform sampler2D	R5_texture4;\n"		// Light Specular (RGB)

"uniform vec4		R5_clipRange;\n"
"uniform vec2		R5_fogRange;\n"		// X = 0-1 fog's start, Y = 0-1 fog's range.

"void main()\n"
"{\n"
"	float depth		= texture2D(R5_texture0, gl_TexCoord[0].xy).r;\n"
"	vec4 matDiff    = texture2D(R5_texture1, gl_TexCoord[0].xy);\n"
"	vec4 matMaps    = texture2D(R5_texture2, gl_TexCoord[0].xy);\n"
"	vec3 lightDiff  = texture2D(R5_texture3, gl_TexCoord[0].xy).rgb;\n"
"	vec3 lightSpec  = texture2D(R5_texture4, gl_TexCoord[0].xy).rgb;\n"
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
"	gl_FragColor = vec4( mix(color, gl_Fog.color.rgb, fogFactor), matDiff.a );\n"
"}\n"};