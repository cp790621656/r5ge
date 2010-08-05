#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred lighting final combination shader
//============================================================================================================

static const char* g_combineDeferred = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// Material Diffuse (RGB)
"uniform sampler2D	R5_texture2;\n"		// Material Specular (RGB) + Self-Illumination (A)
"uniform sampler2D	R5_texture3;\n"		// Light Diffuse (RGB)
"uniform sampler2D	R5_texture4;\n"		// Light Specular (RGB)

"uniform vec4		R5_clipRange;\n"
"uniform vec2		R5_fogRange;\n"		// X = 0-1 fog's start, Y = 0-1 fog's range.

"void main()\n"
"{\n"
"	float depth = texture2D(R5_texture0, gl_TexCoord[0].xy).r;\n"
"	depth = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"

	// Fog's contribution
"	float fogFactor = clamp((depth - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
"	fogFactor = 0.5 * (fogFactor + fogFactor * fogFactor);\n"

"	vec4 matDiff    = texture2D(R5_texture1, gl_TexCoord[0].xy);\n"
"	vec4 matSpec    = texture2D(R5_texture2, gl_TexCoord[0].xy);\n"
"	vec3 lightDiff  = texture2D(R5_texture3, gl_TexCoord[0].xy).rgb;\n"
"	vec3 lightSpec  = texture2D(R5_texture4, gl_TexCoord[0].xy).rgb;\n"

"	vec3 color = matDiff.rgb * matSpec.a +\n"
"				 matDiff.rgb * lightDiff * (1.0 - matSpec.a) +\n"
"				 matSpec.rgb * lightSpec;\n"

"	gl_FragColor = vec4( mix(color, gl_Fog.color.rgb, fogFactor), matDiff.a );\n"
"}\n"};