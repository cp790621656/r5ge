#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Functions for various interpolations
// http://www.gamedev.net/reference/articles/article1497.asp
//============================================================================================================

namespace Interpolation
{
//============================================================================================================
// Vector to take 'start' to 'end'
// This is only here for consistency purposes to match the quaternion function below
//============================================================================================================

inline Vector3f GetDifference (const Vector3f& start, const Vector3f& end)
{
	return (end - start);
}

//============================================================================================================
// Quat to take 'start' to 'end'
//============================================================================================================

inline Quaternion GetDifference (const Quaternion& start, const Quaternion& end)
{
	Quaternion q (	start.w * end.x - start.x * end.w - start.y * end.z + start.z * end.y,
					start.w * end.y - start.y * end.w - start.z * end.x + start.x * end.z,
					start.w * end.z - start.z * end.w - start.x * end.y + start.y * end.x,
					start.w * end.w + start.x * end.x + start.y * end.y + start.z * end.z );

	// Always choose the shortest path
	if (start.Dot(end) < 0) q.Flip();
	return q;
}

//============================================================================================================
// Basic linear interpolation
//============================================================================================================

template <typename Real>
Real Linear (const Real& start, const Real& end, float factor) 
{
	return (start * (1.0f - factor)) + (end * factor);
}

//============================================================================================================
// Cosine interpolation
//============================================================================================================

template <typename Real>
Real Cosine (const Real& current, const Real& next, float factor)
{
	factor = (1.0f - Float::Cos(factor * PI)) * 0.5f;
	return (current * (1.0f - factor) + next * factor);
}

//============================================================================================================
// Simplified cosine interpolation for when 'current' is 0, and 'to' is 1
//============================================================================================================

inline float Cosine (float val) { return (1.0f - Float::Cos(val * PI)) * 0.5f; }

//============================================================================================================
// Cubic interpolation
// 13 operations for floats
//============================================================================================================

template <typename Real>
Real Cubic (const Real& previous, const Real& current, const Real& next, const Real& future, float factor)
{
	float f2(factor * factor);

	Real v0 (future - next - previous + current);
	Real v1 (previous - current - v0);
	Real v2 (next - previous);

	return	v0 * (f2 * factor) +
			v1 * f2 +
			v2 * factor +
			current;
}

//============================================================================================================
// Hermite interpolation (ease in, ease out)
//============================================================================================================

template <typename Real>
Real Hermite (const Real& v0, const Real& v1, float factor)
{
	factor = factor * factor * (3.0f - 2.0f * factor);
	return v0 * (1.0f - factor) + v1 * factor;
}

//============================================================================================================
// Simplified Hermite interpolation
// 22 operations for floats
//============================================================================================================

template <typename Real>
Real Hermite (const Real& previous, const Real& current, const Real& next, const Real& future, float factor)
{
	float f2 = factor * factor;
	float f3 = f2 * factor;

	Real tan0 (next - previous);
	Real tan1 (future - current);

	float f32 = f3 * 2.0f;
	float f23 = f2 * 3.0f;

	float a0  = f32 - f23 + 1.0f;
	float a1  = f23 - f32;
	float a2  = (f3 - f2 * 2.0f + factor) * 0.5f;
	float a3  = (f3 - f2) * 0.5f;

	return current * a0 + next * a1 + tan0 * a2 + tan1 * a3;
}

//============================================================================================================
// Full Hermite interpolation
//============================================================================================================

template <typename Real>
Real Hermite (const Real& previous, const Real& current, const Real& next, const Real& future,
   float factor, float tension, float bias)
{
	float f2 = factor * factor;
	float f3 = f2 * factor;

	Real tan0
	(
		(current - previous) * ((1.0f + bias) * (1.0f - tension)) +
		(next - current)	 * ((1.0f - bias) * (1.0f - tension))
	);
	
	Real tan1
	(
		(next - current) * ((1.0f + bias) * (1.0f - tension)) +
		(future - next)  * ((1.0f - bias) * (1.0f - tension))
	);

	float f32 = f3 * 2.0f;
	float f23 = f2 * 3.0f;

	float a0  = f32 - f23 + 1.0f;
	float a1  = f23 - f32;
	float a2  = (f3 - f2 * 2.0f + factor) * 0.5f;
	float a3  = (f3 - f2) * 0.5f;

	return current * a0 + next * a1 + tan0 * a2 + tan1 * a3;
}

//============================================================================================================
// Hermite spline tangent for a float-based spline
//============================================================================================================

inline float GetHermiteTangent (float distance0, float distance1, float duration)
{
	return (distance0 + distance1) / duration * 0.75f;
}

//============================================================================================================

inline float GetHermiteTangent (float distance0, float distance1, float duration0, float duration1)
{
	return (distance0 / duration0 + distance1 / duration1) * 0.75f;
}

//============================================================================================================
// Calculates a hermite spline tangent at the given point using 3 vectors and two duration factors.
//============================================================================================================

inline Vector3f GetHermiteTangent (const Vector3f& distance0, const Vector3f& distance1, const float duration0, float duration1)
{
	return distance0 * (0.75f / duration0) + distance1 * (0.75f / duration1);
}

//============================================================================================================
// Hermite spline interpolation with the provided tangents
//============================================================================================================

template <typename Real>
Real Hermite (const Real &pos0, const Real &pos1, const Real& tan0, const Real& tan1, float factor, float duration)
{
	float f2  = factor * factor;
	float f3  = f2 * factor;
	float f32 = f3 * 2.0f;
	float f23 = f2 * 3.0f;

	float a0  = f32 - f23 + 1.0f;
	float a1  = f23 - f32;
	float a2  = f3 - f2 * 2.0f + factor;
	float a3  = f3 - f2;

	return   pos0 * a0 +
			 pos1 * a1 +
			(tan0 * a2 +
			 tan1 * a3) * duration;
}

//============================================================================================================
// Optimized function for cases where val0 = 0, and val1 = 1
//============================================================================================================

template <typename Real>
Real Hermite (const Real& tan0, const Real& tan1, float factor, float duration)
{
	float f2 = factor * factor;
	float f3 = f2 * factor;
	return (f2 * 3.0f - f3 * 2.0f) + (tan0 * (f3 - f2 * 2.0f + factor) + tan1 * (f3 - f2)) * duration;
}

//============================================================================================================
// Uniform keyframe Catmull-Rom spline interpolation
//============================================================================================================

Vector3f CatmullRom (const Vector3f &previous, const Vector3f &start, const Vector3f &end, const Vector3f& future, float factor);

//============================================================================================================
// Uniform keyframe spline interpolation that's missing a previous point
//============================================================================================================

Vector3f CatmullRomStart (const Vector3f &start, const Vector3f &end, const Vector3f& future, float factor);

//============================================================================================================
// Uniform keyframe spline interpolation without the future point
//============================================================================================================

Vector3f CatmullRomEnd (const Vector3f &previous, const Vector3f &start, const Vector3f &end, float factor);

//============================================================================================================
// Returns a point along a 1 control point bezier curve
//============================================================================================================

Vector3f Bezier (const Vector3f& vStart, const Vector3f& vControl, const Vector3f& vEnd, float fTime);

//============================================================================================================
// Returns a point along a 2 control point bezier curve
//============================================================================================================

Vector3f Bezier (const Vector3f& vStart, const Vector3f& vControl1, const Vector3f& vControl2, const Vector3f& vEnd, float factor);

//============================================================================================================
// Linear interpolation for quaternions -- in most cases use SLERP instead
//============================================================================================================

inline Quaternion Lerp (const Quaternion& current, const Quaternion& to, float factor)
{
	Quaternion q ( current + (to - current) * factor );
	q.Normalize();
	return q;
}

//============================================================================================================
// Spherical linear interpolation
//============================================================================================================

Quaternion Slerp (const Quaternion& q0, const Quaternion& q1, float factor);

//============================================================================================================
// Finds the control quaternion for the specified quaternion to be used in SQUAD
//============================================================================================================

Quaternion GetSquadControlRotation (const Quaternion& previous, const Quaternion& current, const Quaternion& future);

//============================================================================================================
// Spherical Cubic interpolation -- ctrlFrom and ctrlTo are control quaternions
//============================================================================================================

Quaternion Squad (const Quaternion &current, const Quaternion &to, const Quaternion &ctrlFrom, const Quaternion &ctrlTo, float factor);

//============================================================================================================
// Bilinear texture filtering with tiled behavior
//============================================================================================================

template <typename Real>
Real BilinearTile (const Real* buffer, uint width, uint height, float x, float y)
{
	x *= width;
	y *= height;

	float fx = Float::Floor(x);
	float fy = Float::Floor(y);

	x -= fx;
	y -= fy;

	int ix = Float::RoundToInt(fx);
	int iy = Float::RoundToInt(fy);

	float invX = 1.0f - x;
	float invY = 1.0f - y;

	uint x0  = WrapIndex( ix,	 width );
	uint x1  = WrapIndex( ix + 1, width );
	uint y0w = WrapIndex( iy,	 height ) * width;
	uint y1w = WrapIndex( iy + 1, height ) * width;
	
	return	(buffer[x0 + y0w] * invX + buffer[x1 + y0w] * x) * invY +
			(buffer[x0 + y1w] * invX + buffer[x1 + y1w] * x) * y;
}

//============================================================================================================
// Bilinear texture filtering with clamp-to-edge behavior
//============================================================================================================

template <typename Real>
Real BilinearClamp (const Real* buffer, uint width, uint height, float x, float y)
{
	x *= (width  - 1);
	y *= (height - 1);

	float fx = Float::Floor(x);
	float fy = Float::Floor(y);

	x -= fx;
	y -= fy;

	int ix = Float::RoundToInt(fx);
	int iy = Float::RoundToInt(fy);

	float invX = 1.0f - x;
	float invY = 1.0f - y;

	uint x0  = ClampIndex( ix,	  width  );
	uint x1  = ClampIndex( ix + 1, width  );
	uint y0w = ClampIndex( iy,	  height ) * width;
	uint y1w = ClampIndex( iy + 1, height ) * width;
	
	return	(buffer[x0 + y0w] * invX + buffer[x1 + y0w] * x) * invY +
			(buffer[x0 + y1w] * invX + buffer[x1 + y1w] * x) * y;
}

//============================================================================================================
// Bicubic texture filtering with tiled behavior
//============================================================================================================

template <typename Real>
Real BicubicTile (const Real* buffer, uint width, uint height, float x, float y)
{
	x *= width;
	y *= height;

	float fx = Float::Floor(x);
	float fy = Float::Floor(y);

	x -= fx;
	y -= fy;

	int ix = Float::RoundToInt(fx);
	int iy = Float::RoundToInt(fy);

	uint x0  = WrapIndex( ix - 1, width  );
	uint x1  = WrapIndex( ix,	 width  );
	uint x2  = WrapIndex( ix + 1, width  );
	uint x3  = WrapIndex( ix + 2, width  );
	uint y0w = WrapIndex( iy - 1, height ) * width;
	uint y1w = WrapIndex( iy,	 height ) * width;
	uint y2w = WrapIndex( iy + 1, height ) * width;
	uint y3w = WrapIndex( iy + 2, height ) * width;

	Real v0 = Interpolation::Cubic(buffer[x0 + y0w], buffer[x1 + y0w], buffer[x2 + y0w], buffer[x3 + y0w], x);
	Real v1 = Interpolation::Cubic(buffer[x0 + y1w], buffer[x1 + y1w], buffer[x2 + y1w], buffer[x3 + y1w], x);
	Real v2 = Interpolation::Cubic(buffer[x0 + y2w], buffer[x1 + y2w], buffer[x2 + y2w], buffer[x3 + y2w], x);
	Real v3 = Interpolation::Cubic(buffer[x0 + y3w], buffer[x1 + y3w], buffer[x2 + y3w], buffer[x3 + y3w], x);

	return Interpolation::Cubic(v0, v1, v2, v3, y);
}

//============================================================================================================
// Bicubic texture filtering with the clamp-to-edge behavior
//============================================================================================================

template <typename Real>
Real BicubicClamp (const Real* buffer, uint width, uint height, float x, float y)
{
	x *= (width  - 1);
	y *= (height - 1);

	float fx = Float::Floor(x);
	float fy = Float::Floor(y);

	x -= fx;
	y -= fy;

	int ix = Float::RoundToInt(fx);
	int iy = Float::RoundToInt(fy);

	uint x0  = ClampIndex( ix - 1, width  );
	uint x1  = ClampIndex( ix,	  width  );
	uint x2  = ClampIndex( ix + 1, width  );
	uint x3  = ClampIndex( ix + 2, width  );
	uint y0w = ClampIndex( iy - 1, height ) * width;
	uint y1w = ClampIndex( iy,	  height ) * width;
	uint y2w = ClampIndex( iy + 1, height ) * width;
	uint y3w = ClampIndex( iy + 2, height ) * width;

	Real v0 = Interpolation::Cubic(buffer[x0 + y0w], buffer[x1 + y0w], buffer[x2 + y0w], buffer[x3 + y0w], x);
	Real v1 = Interpolation::Cubic(buffer[x0 + y1w], buffer[x1 + y1w], buffer[x2 + y1w], buffer[x3 + y1w], x);
	Real v2 = Interpolation::Cubic(buffer[x0 + y2w], buffer[x1 + y2w], buffer[x2 + y2w], buffer[x3 + y2w], x);
	Real v3 = Interpolation::Cubic(buffer[x0 + y3w], buffer[x1 + y3w], buffer[x2 + y3w], buffer[x3 + y3w], x);

	return Interpolation::Cubic(v0, v1, v2, v3, y);
}

//============================================================================================================
// Hermite texture filtering with tiled behavior
//============================================================================================================

template <typename Real>
Real HermiteTile (const Real* buffer, uint width, uint height, float x, float y)
{
	x *= width;
	y *= height;

	float fx = Float::Floor(x);
	float fy = Float::Floor(y);

	x -= fx;
	y -= fy;

	int ix = Float::RoundToInt(fx);
	int iy = Float::RoundToInt(fy);

	uint x0  = WrapIndex( ix - 1, width  );
	uint x1  = WrapIndex( ix,	 width  );
	uint x2  = WrapIndex( ix + 1, width  );
	uint x3  = WrapIndex( ix + 2, width  );
	uint y0w = WrapIndex( iy - 1, height ) * width;
	uint y1w = WrapIndex( iy,	 height ) * width;
	uint y2w = WrapIndex( iy + 1, height ) * width;
	uint y3w = WrapIndex( iy + 2, height ) * width;

	Real v0 = Interpolation::Hermite(buffer[x0 + y0w], buffer[x1 + y0w], buffer[x2 + y0w], buffer[x3 + y0w], x);
	Real v1 = Interpolation::Hermite(buffer[x0 + y1w], buffer[x1 + y1w], buffer[x2 + y1w], buffer[x3 + y1w], x);
	Real v2 = Interpolation::Hermite(buffer[x0 + y2w], buffer[x1 + y2w], buffer[x2 + y2w], buffer[x3 + y2w], x);
	Real v3 = Interpolation::Hermite(buffer[x0 + y3w], buffer[x1 + y3w], buffer[x2 + y3w], buffer[x3 + y3w], x);

	return Interpolation::Hermite(v0, v1, v2, v3, y);
}

//============================================================================================================
// Hermite texture filtering with the clamp-to-edge behavior
//============================================================================================================

template <typename Real>
Real HermiteClamp (const Real* buffer, uint width, uint height, float x, float y)
{
	x *= (width  - 1);
	y *= (height - 1);

	float fx = Float::Floor(x);
	float fy = Float::Floor(y);

	x -= fx;
	y -= fy;

	int ix = Float::RoundToInt(fx);
	int iy = Float::RoundToInt(fy);

	uint x0  = ClampIndex( ix - 1, width  );
	uint x1  = ClampIndex( ix,	  width  );
	uint x2  = ClampIndex( ix + 1, width  );
	uint x3  = ClampIndex( ix + 2, width  );
	uint y0w = ClampIndex( iy - 1, height ) * width;
	uint y1w = ClampIndex( iy,	  height ) * width;
	uint y2w = ClampIndex( iy + 1, height ) * width;
	uint y3w = ClampIndex( iy + 2, height ) * width;

	Real v0 = Interpolation::Hermite(buffer[x0 + y0w], buffer[x1 + y0w], buffer[x2 + y0w], buffer[x3 + y0w], x);
	Real v1 = Interpolation::Hermite(buffer[x0 + y1w], buffer[x1 + y1w], buffer[x2 + y1w], buffer[x3 + y1w], x);
	Real v2 = Interpolation::Hermite(buffer[x0 + y2w], buffer[x1 + y2w], buffer[x2 + y2w], buffer[x3 + y2w], x);
	Real v3 = Interpolation::Hermite(buffer[x0 + y3w], buffer[x1 + y3w], buffer[x2 + y3w], buffer[x3 + y3w], x);

	return Interpolation::Hermite(v0, v1, v2, v3, y);
}
}; // namespace Interpolation