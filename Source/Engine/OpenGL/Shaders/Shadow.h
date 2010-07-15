#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Shadow mapping shader -- this shader is incomplete. This only contains the common component.
//============================================================================================================

static const char* g_shadow = {
"uniform vec2 shadowOffset;\n"
"uniform vec4 R5_clipRange;\n"

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
"	depth = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
};