#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Depth-respecting blur shader for ambient occlusion
//============================================================================================================

static const char* g_ssaoBlur = {
"uniform sampler2D	R5_texture0;\n"		// Depth texture
"uniform sampler2D	R5_texture1;\n"		// AO texture
"uniform vec2		R5_pixelSize;\n"
"uniform vec4		R5_clipRange;\n"
"uniform vec2		properties;\n"		// X = focus range, Y = power

// Get the distance to the specified texture coordinate
"float GetDistance (in vec2 texCoord)\n"
"{\n"
"	float depth = texture2D(R5_texture0, texCoord).r;\n"
"	return R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w);\n"
"}\n"

// Get the AO factor at the specified texture coordinate
"float GetFactor (in float origin, in vec2 texCoord)\n"
"{\n"
"	float dist = GetDistance(texCoord);\n"
"	dist = (origin - dist) / properties.x;\n"
"	return min(abs(dist), 1.0);\n"
"}\n"

// Get the AO contribution at the specified texture coordinate
"float GetContribution (in float originalAO, in float dist, in vec2 texCoord)\n"
"{\n"
"	float factor = GetFactor(dist, texCoord);\n"
"	float currentAO = texture2D(R5_texture1, texCoord).r;\n"
"	return mix(currentAO, originalAO, factor);\n"
"}\n"};

//============================================================================================================
// Vertical depth-respecting blur pass
//============================================================================================================

static const char* g_ssaoBlurV = {
"void main()\n"
"{\n"
"	vec2 tc = gl_TexCoord[0].xy;\n"

"	float dist = GetDistance(tc);\n"

"	float o1 = R5_pixelSize.y * 1.5;\n"
"	float o2 = R5_pixelSize.y * 3.5;\n"
"	float o3 = R5_pixelSize.y * 5.5;\n"

"	float ao = texture2D(R5_texture1, tc).r;\n"

"	ao = GetContribution(ao, dist, vec2(tc.x, tc.y - o3)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x, tc.y - o2)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x, tc.y - o1)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x, tc.y + o1)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x, tc.y + o2)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x, tc.y + o3));\n"

"	gl_FragColor = vec4(ao / 6.0);\n"
"}\n"};

//============================================================================================================
// Horizontal depth-respecting blur pass
//============================================================================================================

static const char* g_ssaoBlurH = {
"void main()\n"
"{\n"
"	vec2 tc = gl_TexCoord[0].xy;\n"

"	float dist = GetDistance(tc);\n"

"	float o1 = R5_pixelSize.y * 1.5;\n"
"	float o2 = R5_pixelSize.y * 3.5;\n"
"	float o3 = R5_pixelSize.y * 5.5;\n"

"	float ao = texture2D(R5_texture1, tc).r;\n"

"	ao = GetContribution(ao, dist, vec2(tc.x - o3, tc.y)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x - o2, tc.y)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x - o1, tc.y)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x + o1, tc.y)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x + o2, tc.y)) +\n"
"		 GetContribution(ao, dist, vec2(tc.x + o3, tc.y));\n"

"	gl_FragColor = vec4(ao / 6.0);\n"
"}\n"};

//============================================================================================================
// Screen-space ambient occlusion shader
//============================================================================================================

static const char* g_ssaoSample = {
"uniform sampler2D	R5_texture0;\n"		// Depth
"uniform sampler2D	R5_texture1;\n"		// Normal
"uniform sampler2D	R5_texture2;\n"		// Random

"uniform vec2 R5_pixelSize;\n"			// 0-1 factor size of the pixel
"uniform vec4 R5_clipRange;\n"			// Near/far clipping range
"uniform mat4 R5_projectionMatrix;\n"	// Current projection matrix
"uniform mat4 R5_inverseProjMatrix;\n"	// Inverse projection matrix

"uniform vec2 properties;\n"			// X = focus range, Y = power

// Retrieves depth at the specified texture coordinates
"float GetDepth (in vec2 texCoord)\n"
"{\n"
"	return texture2D(R5_texture0, texCoord).r;\n"
"}\n"

// Calculates the view space position from the specified texture coordinates and depth
"vec3 GetViewPos (in vec2 texCoord)\n"
"{\n"
"	vec4 pos = vec4(texCoord.x, texCoord.y, GetDepth(texCoord), 1.0);\n"
"	pos.xyz = pos.xyz * 2.0 - 1.0;\n"
"	pos = R5_inverseProjMatrix * pos;\n"
"	return pos.xyz / pos.w;\n"
"}\n"

"void main()\n"
"{\n"
"	const int ssaoSamples = 12;\n"

	// Texture coordinate
"	vec2 texCoord = gl_TexCoord[0].xy;\n"

	// View space normal
"	vec3 normal = normalize(texture2D(R5_texture1, texCoord).xyz * 2.0 - 1.0);\n"

	// View space position of the pixel
"	vec3 pos = GetViewPos(texCoord);\n"

	// Random value sampled from the texture in repeated screen coordinates (32 x 32)
"	vec2 modifier = texture2D(R5_texture2, texCoord / R5_pixelSize / 32.0).xy;\n"

"	float dist, visibility = 0.0;\n"
"	vec4 random, screenPos, viewPos = vec4(1.0);\n"

"	for (int i = 0; i < ssaoSamples; i++)\n"
"	{\n"
		// Retrieve a new random vector from the texture
"		random = texture2D(R5_texture2, modifier);\n"

		// Not much point in normalizing -- no visual difference
"		random.xyz = random.xyz * 2.0 - 1.0;\n"

		// Randomize the modifier for the next loop
"		modifier += random.xy;\n"

		// Flip the random vector if it's below the plane
"		if (dot(random.xyz, normal) < 0.0) random.xyz = -random.xyz;\n"

		// Randomly offset view-space position
"		viewPos.xyz = random.xyz * (properties.x * random.w) + pos;\n"

		// Calculate the randomly offset position's screen space coordinates -- second most expensive operation
"		screenPos = R5_projectionMatrix * viewPos;\n"

		// Convert screen space coordinates to 0-1 range and get the depth underneath (most expensive operation)
		// This used to be: (screenPos.xy / screenPos.w * 0.5 + 0.5)
"		dist = GetDepth(screenPos.xy / (screenPos.w * 2.0) + 0.5);\n"

		// Convert the depth to linear form (note that this value is positive, while viewPos.z is negative)
"		dist = R5_clipRange.z / (R5_clipRange.y - dist * R5_clipRange.w);\n"

		// Difference in linear depth relative to the focus range
"		dist = (viewPos.z + dist) / properties.x;\n"

		// We want occlusion to fade out if the depth difference becomes too great.
		// It should be 0% at 0, 100% at -0.5, and 0% again at -1.
"		dist = abs(dist + 1.0);\n"

		// Distance is currently (0 to X) range -- limit it to (0 to 1) range
"		visibility += min(dist, 1.0);\n"
"	}\n"

	// Final occlusion factor
"	gl_FragColor = vec4(pow(visibility / float(ssaoSamples), properties.y));\n"
"}\n"};