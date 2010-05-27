#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Bloom post-processing effect
//============================================================================================================

static const char* g_blurBloom = {
"uniform sampler2D	 R5_texture0;\n"
"uniform vec2		 R5_pixelSize;\n"
"uniform float		 threshold;\n"

"void main()\n"
"{\n"
"	vec2 tc = gl_TexCoord[0].xy;\n"

"	float o1 = R5_pixelSize.x * 0.5;\n"
"	float o2 = R5_pixelSize.x * 2.5;\n"

"	gl_FragColor = max( (max(texture2D(R5_texture0, vec2(tc.x - o2, tc.y)), threshold) +\n"
"		 				 max(texture2D(R5_texture0, vec2(tc.x - o1, tc.y)), threshold) +\n"
"						 max(texture2D(R5_texture0, vec2(tc.x + o1, tc.y)), threshold) +\n"
"						 max(texture2D(R5_texture0, vec2(tc.x + o2, tc.y)), threshold)) * 0.25 - threshold, 0.0 );\n"
"}\n"
};

//============================================================================================================

static const char* g_combineBloom = {
"uniform sampler2D	 R5_texture0;\n"
"uniform sampler2D	 R5_texture1;\n"
"uniform sampler2D	 R5_texture2;\n"
"uniform sampler2D	 R5_texture3;\n"

"void main()\n"
"{\n"
"	vec4 original	=  texture2D(R5_texture0, gl_TexCoord[0].xy);\n"
"	vec4 downsample = (texture2D(R5_texture1, gl_TexCoord[0].xy) +\n"
"					   texture2D(R5_texture2, gl_TexCoord[0].xy) +\n"
"					   texture2D(R5_texture3, gl_TexCoord[0].xy));\n"

"	gl_FragColor = original + downsample;\n"
"}\n"
};