#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#ifndef _MATH_INCLUDE_H
#define _MATH_INCLUDE_H

#include "../../Basic/Include/_All.h"

#include <math.h>
#include "_Define.h"			// Macros and defines used commonly throughout the code (such as HALFPI)

#ifndef _WINDOWS
	#include <stdlib.h>			// Windows has abs() defined in <math.h>
#endif

namespace R5
{
	#include "Basic.h"			// Very basic math functions such as Clamp(), Wrap(), and NarrowPrecision()
	#include "Color3f.h"		// Color3f  -- 3 float color
	#include "Color4f.h"		// Color4f  -- 4 float color
	#include "Color4ub.h"		// Color4ub -- 4 byte color
	#include "Color.h"			// Unified 32-bit as well as 128-bit format color, for optimization purposes
	#include "Vector2i.h"		// 2D short-based vector -- an alternative to using LOWORD() / HIWORD()
	#include "Vector2f.h"		// 2D float-based vector
	#include "Vector3f.h"		// Standard 3D vector
	#include "Vector3d.h"		// Double precision floating point vector
	#include "Quaternion.h"		// Quaternion
	#include "Matrix43.h"		// 4x3 matrix suitable for view and world transformation
	#include "Matrix44.h"		// 4x4 matrix suitable for everything Matrix43 is meant for, plus projection
	#include "Bounds.h"			// Bounding sphere + box for quick viewing frustum checks
	#include "Frustum.h"		// Viewing frustum
	#include "Functions.h"		// Various 3D math functions (Vector, Normalize, Cross, operators, etc)
	#include "Interpolation.h"	// Functions for interpolation -- from linear to spline
	#include "SplineF.h"		// Float spline
	#include "SplineV.h"		// Vector3f spline
	#include "SplineQ.h"		// Quaternion spline
	#include "Shapes.h"			// Geometric shapes generated using math algorithms
	#include "Intersect.h"		// Intersection test functions
	#include "Rectangle.h"		// Basic templated rectangle
};

#endif
