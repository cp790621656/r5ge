#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Depth-of-field post-processing effect
//============================================================================================================

static const char* g_dof = {
"uniform sampler2D	 R5_texture0;\n"	 // Depth
"uniform sampler2D	 R5_texture1;\n"	 // Color
"uniform sampler2D	 R5_texture2;\n"	 // Downsampled and blurred color
"uniform sampler2D	 R5_texture3;\n"	 // Even further downsampled / blurred color
"uniform vec4		 R5_clipRange;\n"	 // Near/far clipping range

// Value 0 = center distance for the depth of field calculations
// Value 1 = how far from the center the original texture starts fading
// Value 2 = how far from the center first downsample becomes 100%
// Value 3 = how far from the center second downsample becomes 100%
"uniform vec4 focusRange;\n"

"void main()\n"
"{\n"
	// Figure out the distance to this pixel
"	float depth = texture2D(R5_texture0, gl_TexCoord[0].xy).r;\n"
"	float dist	= R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w);\n"

	// Distance from the focal point
"	dist = abs(dist - focusRange.x);\n"

	// Distance from the first edge
"	float factor0 = clamp((focusRange.z - dist) / (focusRange.z - focusRange.y), 0.0, 1.0);\n"

	// Distance from the second edge
"	float factor1 = clamp((focusRange.w - dist) / (focusRange.w - focusRange.z), 0.0, 1.0);\n"

"	vec3 original	 = texture2D(R5_texture1, gl_TexCoord[0].xy).rgb;\n"
"	vec3 downsample0 = texture2D(R5_texture2, gl_TexCoord[0].xy).rgb;\n"
"	vec3 downsample1 = texture2D(R5_texture3, gl_TexCoord[0].xy).rgb;\n"

"	vec3 final = mix( mix(downsample1, downsample0, factor1 * factor1), original, factor0);\n"
"	gl_FragColor = vec4(final, 1.0);\n"
"}\n"
};
