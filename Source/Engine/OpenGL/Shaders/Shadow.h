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
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	vec4 worldPos = vec4(texCoord.x, texCoord.y, depth, 1.0);\n"

"	vec4 pos4;\n"
"	vec3 shadow;\n"

"	pos4 = shadowMatrix0 * worldPos;\n"
"	shadow.x = Sample(R5_texture1, pos4.xyz / pos4.w);\n"

"	pos4 = shadowMatrix1 * worldPos;\n"
"	shadow.y = Sample(R5_texture2, pos4.xyz / pos4.w);\n"

"	pos4 = shadowMatrix2 * worldPos;\n"
"	shadow.z = Sample(R5_texture3, pos4.xyz / pos4.w);\n"

"	gl_FragColor = vec4(min(shadow.x, min(shadow.y, shadow.z)));\n"

//"	const vec4 cr = vec4(1.0, 200.0, 200.0, 199.0);\n"
//"	float eyeZ = (cr.x - cr.z / (depth * cr.w - cr.y)) / cr.w;\n"
//"	float a = max(0.0, min(1.0, eyeZ / 0.222222) - 0.9) * 10.0;\n"
//"	a *= a;\n"
//"	a *= a;\n"
//"	float b = max(0.0, min(1.0, eyeZ / 0.555555) - 0.9) * 10.0;\n"
//"	b *= b;\n"
//"	b *= b;\n"
//"	gl_FragColor = vec4(mix(mix(shadow.x, shadow.y, a), shadow.z, b));\n"
"}"
};