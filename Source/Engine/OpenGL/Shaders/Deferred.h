#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred lighting final combination shader
//============================================================================================================

static const char* g_deferredCombine = {
"uniform sampler2D   R5_texture0;\n"    // Material Diffuse (RGB)
"uniform sampler2D   R5_texture1;\n"    // Material Specular (RGB) + Self-Illumination (A)
"uniform sampler2D   R5_texture2;\n"    // Light Diffuse (RGB)
"uniform sampler2D   R5_texture3;\n"    // Light Specular (RGB)

"void main()\n"
"{\n"
"	vec4 matDiff    = texture2D(R5_texture0, gl_TexCoord[0].xy);\n"
"	vec4 matSpec    = texture2D(R5_texture1, gl_TexCoord[0].xy);\n"
"	vec4 lightDiff  = texture2D(R5_texture2, gl_TexCoord[0].xy);\n"
"	vec4 lightSpec  = texture2D(R5_texture3, gl_TexCoord[0].xy);\n"

"	gl_FragColor = vec4(matDiff.rgb * matSpec.a +\n"
"						matDiff.rgb * lightDiff.rgb * (1.0 - matSpec.a) +\n"
"						matSpec.rgb * lightSpec.rgb, matDiff.a);\n"
"}\n"};