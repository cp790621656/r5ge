#include "../Include/_All.h"

namespace R5
{
namespace Interpolation
{
// Hermite spline interpolation with the provided tangents
Vector3f Hermite(const Vector3f &pos0, const Vector3f &pos1, const Vector3f& tan0, const Vector3f& tan1, float factor, float duration)
{
	float factor2 = factor  * factor;
	float factor3 = factor2 * factor;

	return   pos0 * (2.0f * factor3 - 3.0f * factor2 + 1		) +
			 pos1 * (3.0f * factor2 - 2.0f * factor3			) +
			(tan0 * (		factor3 - 2.0f * factor2 + factor	) +
			 tan1 * (		factor3 -	     factor2			)) * duration;
}

// Optimized function for cases where val0 = 0, and val1 = 1
float Hermite(float tan0, float tan1, float factor, float duration)
{
	float factor2 = factor  * factor;
	float factor3 = factor2 * factor;

	return	 (3.0f * factor2 - 2.0f * factor3			) +
		(tan0 * (    factor3 - 2.0f * factor2 + factor	) +
		 tan1 * (    factor3 -		  factor2			)) * duration;
}

// Uniform keyframe Catmull-Rom spline interpolation
Vector3f CatmullRom(const Vector3f &past, const Vector3f &start, const Vector3f &end, const Vector3f& future, float factor)
{
	return start +  (end - past +
					(past  * 2.0f - start * 5.0f + end * 4.0f - future) * factor +
					(start * 3.0f - end   * 3.0f - past		  + future) * (factor * factor)) * (factor * 0.5f);
}

// Uniform keyframe spline interpolation that's missing a past point
Vector3f CatmullRomStart(const Vector3f &start, const Vector3f &end, const Vector3f& future, float factor)
{
	return start +  (end - start +
					(end * 4.0f - start * 3.0f - future) * factor +
					(start * 2.0f - end * 3.0f + future) * (factor * factor)) * (factor * 0.5f);
}

// Uniform keyframe spline interpolation without the future point
Vector3f CatmullRomEnd(const Vector3f &past, const Vector3f &start, const Vector3f &end, float factor)
{
	return start +  (end - past +
					(past  * 2.0f - start * 5.0f + end * 3.0f) * factor +
					(start * 3.0f - end   * 2.0f - past) * (factor * factor)) * (factor * 0.5f);
}

// Returns a point along a 1 control point bezier curve
Vector3f Bezier (const Vector3f& v0, const Vector3f& vControl, const Vector3f& v1, float factor)
{
	float invFactor		= 1.0f - factor;
	float inv2			= invFactor * invFactor;
	float factor2		= factor * factor;
	float factorInv2	= factor * invFactor * 2.0f;

	return Vector3f(inv2 * v0.x + factorInv2 * vControl.x + factor2 * v1.x,
					inv2 * v0.y + factorInv2 * vControl.y + factor2 * v1.y,
					inv2 * v0.z + factorInv2 * vControl.z + factor2 * v1.z);
}

// Returns a point along a 2 control point bezier curve
Vector3f Bezier (const Vector3f& v0, const Vector3f& a, const Vector3f& b, const Vector3f& v1, float factor)
{
	float invFactor		= 1.0f - factor;
	float inv3			= invFactor * invFactor * invFactor;
	float factor3		= factor * factor * factor;
	float factor2Inv3	= factor * factor * invFactor * 3.0f;
	float inv2Factor3	= factor * invFactor * invFactor * 3.0f;

	return Vector3f(inv3 * v0.x + inv2Factor3 * a.x + factor2Inv3 * b.x + factor3 * v1.x,
					inv3 * v0.y + inv2Factor3 * a.y + factor2Inv3 * b.y + factor3 * v1.y,
					inv3 * v0.z + inv2Factor3 * a.z + factor2Inv3 * b.z + factor3 * v1.z);
}

// Spherical linear interpolation
Quaternion Slerp (const Quaternion& from, const Quaternion& to, float factor)
{
	float dot = from.Dot(to), theta, sinInv, first, second (1.0f);

	// Choose the shortest path
	if (dot < 0.0f)
	{
		dot = -dot;
		second = -1.0f;
	}

	// If the quaternions are too close together, LERP
	if (dot > FLOAT_INV_TOLERANCE)
	{
		return from + (to - from) * factor;
	}
	
	// Otherwise SLERP
	theta   = Float::Acos(dot);
	sinInv  = 1.0f / Float::Sin(theta);
	first   = Float::Sin((1.0f - factor) * theta) * sinInv;
	second *= Float::Sin(factor * theta) * sinInv;
	
	// Final result is pretty straightforward
	return Quaternion(	first * from.x + second * to.x,
						first * from.y + second * to.y,
						first * from.z + second * to.z,
						first * from.w + second * to.w );
}

// Spherical linear interpolation that does not choose the shortest path (for SQUAD)
Quaternion SlerpNoInvert (const Quaternion& from, const Quaternion& to, float factor)
{
	float dot = from.Dot(to);

	if (Float::Abs(dot) > FLOAT_INV_TOLERANCE)
	{
		return from + (to - from) * factor;
	}
	
	float theta		= Float::Acos(dot);
	float sinInv	= 1.0f / Float::Sin(theta);
	float first		= Float::Sin((1.0f - factor) * theta) * sinInv;
	float second	= Float::Sin(factor * theta) * sinInv;

	return Quaternion(	first * from.x + second * to.x,
						first * from.y + second * to.y,
						first * from.z + second * to.z,
						first * from.w + second * to.w );
}

// Quaternion Logarithm
Quaternion Log(const Quaternion& q)
{
	float a = Float::Acos(q.w), s = Float::Sin(a);

	if (Float::IsZero(s)) return Quaternion( 0, 0, 0, 0 );

	a /= s;

	return Quaternion(	q.x * a,  q.y * a,  q.z * a, 0 );
}

// Quaternion Exponent
Quaternion Exp(const Quaternion& q)
{
	float a = Float::Sqrt(q.x * q.x + q.y * q.y + q.z * q.z),
	      s = Float::Sin(a),
	      c = Float::Cos(a);

	if (Float::IsZero(a))
	{
		return Quaternion( 0.0f, 0.0f, 0.0f, c );
	}

	s /= a;

	return Quaternion( q.x * s,  q.y * s,  q.z * s,  c );
}

// Finds the control quaternion for the specified quaternion to be used in SQUAD
Quaternion GetSquadControlRotation( const Quaternion& past, const Quaternion& current, const Quaternion& future )
{
	Quaternion q ( -current.x, -current.y, -current.z, current.w );
	return current * Exp( (Log(q * past) + Log(q * future)) * -0.25f);
}

// Spherical Cubic interpolation -- ctrlFrom and ctrlTo are control quaternions
Quaternion Squad(const Quaternion &from, const Quaternion &to, const Quaternion &ctrlFrom, const Quaternion &ctrlTo, float factor)
{
	return SlerpNoInvert( SlerpNoInvert(from, to, factor),
						  SlerpNoInvert(ctrlFrom, ctrlTo, factor),
						  factor * 2.0f * (1.0f - factor) );
}
};
};