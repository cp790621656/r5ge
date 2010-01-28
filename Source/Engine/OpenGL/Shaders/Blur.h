#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Smooth gaussian blur shaders (5x5)
//============================================================================================================

static const char* g_blurH = {
"uniform sampler2D	 R5_texture0;\n"
"uniform vec2		 R5_pixelSize;\n"

"void main()\n"
"{\n"
"	vec2 tc = gl_TexCoord[0].xy;\n"

"	float o1 = R5_pixelSize.x * 0.5;\n"
"	float o2 = R5_pixelSize.x * 2.5;\n"

"	gl_FragColor = (texture2D(R5_texture0, vec2(tc.x - o2, tc.y)) +\n"
"					texture2D(R5_texture0, vec2(tc.x - o1, tc.y)) +\n"
"					texture2D(R5_texture0, vec2(tc.x + o1, tc.y)) +\n"
"					texture2D(R5_texture0, vec2(tc.x + o2, tc.y))) * 0.25;\n"
"}\n"
};

//============================================================================================================

static const char* g_blurV = {
"uniform sampler2D	R5_texture0;\n"
"uniform vec2		R5_pixelSize;\n"

"void main()\n"
"{\n"
"	vec2 tc = gl_TexCoord[0].xy;\n"

"	float o1 = R5_pixelSize.y * 0.5;\n"
"	float o2 = R5_pixelSize.y * 2.5;\n"

"	gl_FragColor = (texture2D(R5_texture0, vec2(tc.x, tc.y - o2)) +\n"
"					texture2D(R5_texture0, vec2(tc.x, tc.y - o1)) +\n"
"					texture2D(R5_texture0, vec2(tc.x, tc.y + o1)) +\n"
"					texture2D(R5_texture0, vec2(tc.x, tc.y + o2))) * 0.25;\n"
"}\n"
};