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
"uniform mat4				shadowMatrix;\n"
"uniform vec2				shadowOffset;\n"

"void main()\n"
"{\n"
"	vec2 texCoord = gl_TexCoord[0].xy;\n"
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	vec3 pos = vec3(texCoord.x, texCoord.y, depth);\n"

	// Transform the screen coordinate to light space
"	vec4 pos4 = vec4(pos, 1.0);\n"
"	pos4 = shadowMatrix * pos4;\n"
"	pos = pos4.xyz / pos4.w;\n"

"	float shadowFactor = shadow2D(R5_texture1, pos).r;\n"

"	shadowFactor += shadow2D(R5_texture1, pos + vec3(-shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(R5_texture1, pos + vec3(-shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(R5_texture1, pos + vec3( shadowOffset.x, -shadowOffset.y, 0.0)).r;\n"
"	shadowFactor += shadow2D(R5_texture1, pos + vec3( shadowOffset.x,  shadowOffset.y, 0.0)).r;\n"
"	shadowFactor *= 0.2;\n"

"	gl_FragColor = vec4(shadowFactor);\n"
"}"
};