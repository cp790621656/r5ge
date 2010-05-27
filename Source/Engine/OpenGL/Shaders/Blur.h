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


//============================================================================================================
// TODO: Improve the blur itself. If this is the only thing that's needed for shadows, why have everything else?
//============================================================================================================

static const char* g_combineAverageBlur = {
"uniform sampler2D	R5_texture1;\n"

"void main()\n"
"{\n"
"	float a = pow(texture2D(R5_texture1, gl_TexCoord[0].xy).r, 2.0);\n"
"	gl_FragColor = vec4(a);\n"
"}\n"
};

//============================================================================================================

static const char* g_combineMonoBlur = {
"uniform sampler2D	R5_texture0;\n"
"uniform sampler2D	R5_texture1;\n"
"uniform sampler2D	R5_texture2;\n"
"uniform sampler2D	R5_texture3;\n"
"uniform float		sharpness;\n"

"void main()\n"
"{\n"
"	float intensity =	(texture2D(R5_texture0, gl_TexCoord[0].xy).r +\n"
"						 texture2D(R5_texture1, gl_TexCoord[0].xy).r +\n"
"						 texture2D(R5_texture2, gl_TexCoord[0].xy).r +\n"
"						 texture2D(R5_texture3, gl_TexCoord[0].xy).r) * 0.25;\n"
"	intensity = pow(intensity, sharpness);\n"
"	gl_FragColor = vec4(intensity);\n"
"}\n"
};
