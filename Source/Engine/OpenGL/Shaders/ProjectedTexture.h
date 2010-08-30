#pragma once

//============================================================================================================
//					R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Built-in shader for a projected texture
//============================================================================================================

static const char* g_projectedTexture = {
"uniform sampler2D R5_texture0;\n"	// View-space depth texture
"uniform sampler2D R5_texture1;\n"	// Projected diffuse texture

"uniform vec2 R5_pixelSize;\n"
"uniform mat4 g_mat;\n"				// Inverse world-view-projection matrix
"uniform vec4 g_color;\n"			// Object's color (ATI clamps material color within 0-1 range)

//============================================================================================================
// Gets the view space position at the specified texture coordinates
//============================================================================================================

"vec3 GetWorldPos (in vec2 screenTC)\n"
"{\n"
"	float depth = texture2D(R5_texture0, screenTC).r;\n"
"	vec4 pos = vec4(screenTC.x, screenTC.y, depth, 1.0);\n"
"	pos.xyz = pos.xyz * 2.0 - 1.0;\n"
"	pos = g_mat * pos;\n"
"	return pos.xyz / pos.w;\n"
"}\n"

//============================================================================================================

"void main()\n"
"{\n"
	// This pixel's texture coordinates
"	vec2 screenTC = gl_FragCoord.xy * R5_pixelSize;\n"

	// This pixel's relative-to-center position
"	vec3 pos = GetWorldPos(screenTC);\n"
"	float alpha = max(abs(pos.x), max(abs(pos.y), abs(pos.z)));\n"

	// Discard fragments that lie outside of the box
"	if (alpha > 1.0) discard;\n"

	// Make alpha more focused in the center
"	alpha = 1.0 - pow(alpha, 4.0);\n"

    // Sample the decal texture
"	vec4 projDiffuse = texture2D(R5_texture1, pos.xz * 0.5 + 0.5) * g_color;\n"
};

//============================================================================================================
// Add / subtract
//============================================================================================================

static const char* g_projectedTextureAddSubtract = {
"	gl_FragColor = projDiffuse * alpha;\n"
"}"
};

//============================================================================================================
// Replace
//============================================================================================================

static const char* g_projectedTextureReplace = {
"	projDiffuse.a *= alpha;\n"
"	gl_FragColor = projDiffuse;\n"
"}"
};

//============================================================================================================
// Modulate
//============================================================================================================

static const char* g_projectedTextureModulate = {
"	projDiffuse.rgb = mix(vec3(1.0, 1.0, 1.0), projDiffuse.rgb, alpha);\n"
"	gl_FragColor = projDiffuse;\n"
"}"
};