#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================

// Basic definitions
#define FLOAT_TOLERANCE			0.000001f
#define FLOAT_INV_TOLERANCE		0.999999f
#define PI						3.1415926535897932f
#define TWOPI					6.28318530718f
#define HALFPI					1.570796326795f

// Macros
#define MAKEUINT(a, b)			((uint)a ^ ((uint)b << 16))
#define DEG2RAD(Degrees)		(Degrees * 0.01745329252f)
#define RAD2DEG(Rads)			(Rads * 57.29577951f)