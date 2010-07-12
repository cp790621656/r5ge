#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Shadow mapping shader
//============================================================================================================

static const char* g_shadow = {
"uniform sampler2D			R5_texture0;\n"		// Camera's depth texture
"uniform sampler2DShadow	R5_texture1;\n"		// Lights' depth texture
"uniform sampler2DShadow	R5_texture2;\n"		// Lights' depth texture
"uniform sampler2DShadow	R5_texture3;\n"		// Lights' depth texture

"uniform mat4 shadowMatrix0;\n"
"uniform mat4 shadowMatrix1;\n"
"uniform mat4 shadowMatrix2;\n"
"uniform vec2 shadowOffset;\n"

"float GetMax (vec3 v)\n"
"{\n"
"	v = abs(vec3(0.5) - v);\n"
"	return 2.0 * max(v.x, max(v.y, v.z));\n"
"}\n"

"float Sample (sampler2DShadow tex, vec3 pos)\n"
"{\n"
"	float shadowFactor = shadow2D(tex, pos).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3(-shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3(-shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3( shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3( shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
"	shadowFactor *= 0.2;\n"
"	return shadowFactor;\n"
"}\n"

"void main()\n"
"{\n"
"	vec2 texCoord = gl_TexCoord[0].xy;\n"
"	float shadowFactor = 1.0;\n"
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	vec4 worldPos = vec4(texCoord.x, texCoord.y, depth, 1.0);\n"

	// First split -- transform the coordinate to light space and sample the shadow texture
"	vec4 pos4 = shadowMatrix2 * worldPos;\n"
"	vec3 pos = pos4.xyz / pos4.w;\n"
"	float factor = GetMax(pos);\n"
"	vec3 result = vec3(0.0);\n"

"	if (factor < 1.0)\n"
"	{\n"
"		result.b = 1.0;\n"
"		shadowFactor = Sample(R5_texture3, pos);\n"
"	}\n"

	// Second split
"	pos4 = shadowMatrix1 * worldPos;\n"
"	pos = pos4.xyz / pos4.w;\n"
"	factor = GetMax(pos);\n"

"	if (factor < 1.0)\n"
"	{\n"
"		factor *= factor;\n"
"		factor *= factor;\n"
"		factor *= factor;\n"
"		result.g = 1.0;\n"
"		float contribution = Sample(R5_texture2, pos);\n"
"		shadowFactor = (result.b == 1.0) ? mix(contribution, shadowFactor, factor) : contribution;\n"
"	}\n"

	// Third split
"	pos4 = shadowMatrix0 * worldPos;\n"
"	pos = pos4.xyz / pos4.w;\n"
"	factor = GetMax(pos);\n"

"	if (factor < 1.0)\n"
"	{\n"
"		factor *= factor;\n"
"		factor *= factor;\n"
"		factor *= factor;\n"
"		result.r = 1.0;\n"
"		float contribution = Sample(R5_texture1, pos);\n"
"		shadowFactor = (result.g == 1.0) ? mix(contribution, shadowFactor, factor) : contribution;\n"
"	}\n"

"	gl_FragColor = vec4(result, shadowFactor);\n"
"}"
};