#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Smooth gaussian blur shaders
// Author: Michael Lyashenko
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
// Depth-respecting blur shader, used for shadows and SSAO
//============================================================================================================

static const char* g_depthRespectingBlur = {
"uniform sampler2D R5_texture0;\n"	// Depth texture
"uniform sampler2D R5_texture1;\n"	// Shadow lightmap

"uniform vec4 R5_clipRange;\n"
"uniform vec2 threshold;\n"			// X = pixel size threshold, Y = depth variance threshold

// Get the distance to the specified texture coordinate
"float GetDistance (in vec2 texCoord)\n"
"{\n"
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	return R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w);\n"
"}\n"

// Contribution value is depth-dependent
"float GetContribution (in float originalDistance, in float originalValue, in vec2 texCoord)\n"
"{\n"
"	float current = texture2D(R5_texture1, texCoord).r;\n"
"	float factor = min(abs((originalDistance - GetDistance(texCoord)) / threshold.y), 1.0);\n"
"	return mix(current, originalValue, factor);\n"
"}\n"

// Performs a depth-respecting blur
"void main()\n"
"{\n"
"	vec2 tc = gl_TexCoord[0].xy;\n"
"	float originalDistance = GetDistance(tc);\n"
"	float originalValue = texture2D(R5_texture1, tc).r;\n"

 	// The closer to the camera the larger is the area we can sample
"	float strength = (1.0 - originalDistance / R5_clipRange.w);\n"
"	float pixelSize = threshold.x * max(0.75, strength * strength * 3.0);\n"

	// Pixel offsets
"	float o2 = pixelSize + pixelSize;\n"
"	float o3 = o2 + pixelSize;\n"
};

//============================================================================================================

static const char* g_depthRespectingBlurH = {
"	float final =\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x - o3, tc.y)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x + o3, tc.y)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x - o2, tc.y)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x + o2, tc.y)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x - pixelSize, tc.y)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x + pixelSize, tc.y));\n"

"	gl_FragColor = vec4(final / 6.0);\n"
"}"
};

//============================================================================================================

static const char* g_depthRespectingBlurV = {
"float final =\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y - o3)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y + o3)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y - o2)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y + o2)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y - pixelSize)) +\n"
"		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y + pixelSize));\n"

"	gl_FragColor = vec4(final / 6.0);\n"
"}"
};