#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Shadow mapping shader -- this shader is incomplete. This only contains the common component.
// Author: Michael Lyashenko
//============================================================================================================

static const char* g_shadow = {
"uniform vec2 shadowOffset;\n"
"uniform vec4 R5_clipRange;\n"

"float SamplePCF (sampler2DShadow tex, vec3 pos)\n"
"{\n"
"	float shadowFactor = shadow2D(tex, pos).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3(-shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3(-shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3( shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(tex, pos + vec3( shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
"	shadowFactor *= 0.2;\n"
"	return shadowFactor;\n"
"}\n"

//"float Sample (sampler2D tex, vec3 pos)\n"
//"{\n"
//"	float shadowFactor = texture2D(tex, pos.xy).r;\n"
//"	return 1.0 - clamp(pos.z / shadowFactor - 1.005, 0.0, 0.005) / 0.005;\n"
//"}\n"
//
//"float SamplePCF (sampler2D tex, vec3 pos)\n"
//"{\n"
//"	float shadowFactor = Sample(tex, pos);\n"
//"	shadowFactor += Sample(tex, pos + vec3(-shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
//"	shadowFactor += Sample(tex, pos + vec3(-shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
//"	shadowFactor += Sample(tex, pos + vec3( shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
//"	shadowFactor += Sample(tex, pos + vec3( shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
//"	return shadowFactor * 0.2;\n"
//"}\n"

"void main()\n"
"{\n"
"	vec2 texCoord = gl_TexCoord[0].xy;\n"
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	vec4 worldPos = vec4(texCoord.x, texCoord.y, depth, 1.0);\n"
"	depth = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
};