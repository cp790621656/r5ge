#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred lighting final combination shader
//============================================================================================================

static const char* g_combineDeferred = {
"R5_FRAGMENT_SHADER\n"
"{\n"
"	float2 tc = GetPixelTexCoords();\n"

"	float depth		= Sample2D(0, tc).r;\n"
"	half4 matDiff   = Sample2D(1, tc);\n"
"	half4 matMaps   = Sample2D(2, tc);\n"
"	half3 lightDiff = Sample2D(3, tc).rgb;\n"
"	half3 lightSpec = Sample2D(4, tc).rgb;\n"

"	half3 specular	= lightSpec * matMaps.r;\n"
"	half3 color		= mix(lightDiff * matDiff.rgb, matDiff.rgb, matMaps.b) +\n"
"					  mix(specular, specular * matDiff.rgb, matMaps.g);\n"

// AO visualization:
//"	color = half3(matMaps.a, matMaps.a, matMaps.a);\n"

"	float fogFactor = GetFogFactor(depth);\n"
"	R5_finalColor[0] = half4( mix(color, R5_fogColor.rgb, fogFactor), matDiff.a );\n"
"}\n"};