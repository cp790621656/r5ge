#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Externalized for speed
//============================================================================================================

const float to1 = 256.0f;
const float to2 = to1 * to1;
const float to3 = to2 * to1;
const float to4 = to3 * to1;

const float over1 = 1.0f / to1;
const float over2 = 1.0f / to2;
const float over3 = 1.0f / to3;
const float over4 = 1.0f / to4;

//==========================================================================================================
// Inline function that converts a hex character to its decimal representation
//==========================================================================================================

uint HexToDecimal (char ch)
{
	switch (ch)
	{
		case '0': return 0x0;
		case '1': return 0x1;
		case '2': return 0x2;
		case '3': return 0x3;
		case '4': return 0x4;
		case '5': return 0x5;
		case '6': return 0x6;
		case '7': return 0x7;
		case '8': return 0x8;
		case '9': return 0x9;
		case 'a': case 'A': return 0xA;
		case 'b': case 'B': return 0xB;
		case 'c': case 'C': return 0xC;
		case 'd': case 'D': return 0xD;
		case 'e': case 'E': return 0xE;
		case 'f': case 'F': return 0xF;
	}
	return 0xF;
}

//============================================================================================================
// Sets the color based on the specified text value in RRGGBBAA hex format
//============================================================================================================

void Color4ub::SetByHexString (const char* text, uint length)
{
	if (length < 2) return;		r = (HexToDecimal(text[0]) << 4) | HexToDecimal(text[1]);
	if (length < 4) return;		g = (HexToDecimal(text[2]) << 4) | HexToDecimal(text[3]);
	if (length < 6) return;		b = (HexToDecimal(text[4]) << 4) | HexToDecimal(text[5]);
	if (length < 8)	return;		a = (HexToDecimal(text[6]) << 4) | HexToDecimal(text[7]);
}

//============================================================================================================
// Packs a float into 4-component RGBA, equivalent of the GLSL shader:
//============================================================================================================
//vec4 PackFloat (in float val)
//{
//	const vec4 multiplier = vec4(16777216.0, 65535.0, 256.0, 1.0);
//	const vec4 bitMask = vec4(0.0, 0.00390625, 0.00390625, 0.00390625);
//	vec4 ret = fract(multiplier * val);
//	return ret - ret.xxyz * bitMask;
//}
//============================================================================================================

void Color4ub::PackFloat (float w)
{
	ASSERT ( !(w < 0.0f || w > 1.0f), "Invalid argument, float must be within 0 to 1 range" );

	float x = w * to3;
	float y = w * to2;
	float z = w * to1;

	x -= Float::Floor(x);
	y -= Float::Floor(y);
	z -= Float::Floor(z);

	w -= z * over1;
	z -= y * over1;
	y -= x * over1;

	uint ia = Float::RoundToUInt(w * to1);

	if (ia > 255)
	{
		mVal = -1;
	}
	else
	{
		r = Float::RoundToByte(x * to1);
		g = Float::RoundToByte(y * to1);
		b = Float::RoundToByte(z * to1);
		a = (byte)ia;
	}
}

//============================================================================================================
// Unpacks a previously packed float, equivalent of the GLSL shader:
//============================================================================================================
//float UnpackFloat (in vec4 val)
//{
//	const vec4 multiplier = vec4(1.0 / 16777216.0, 1.0 / 65535.0, 1.0 / 256.0, 1.0);
//	return dot(val, multiplier);
//}
//============================================================================================================

float Color4ub::UnpackFloat() const
{
	return over4 * r + over3 * g + over2 * b + over1 * a;
}