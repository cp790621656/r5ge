#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
//  Standard 3D Math functions
// Author: Michael Lyashenko
//============================================================================================================

inline Vector3f Cross (const Vector3f& v0, const Vector3f& v1)
{
	return Vector3f((v0.y * v1.z) - (v0.z * v1.y),
		            (v0.z * v1.x) - (v0.x * v1.z),
					(v0.x * v1.y) - (v0.y * v1.x));
}

//============================================================================================================

inline float Magnitude (const Vector3f& v)
{
	return Float::Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

//============================================================================================================

inline float Dot (const Vector3f& v1, const Vector3f& v2)
{
	return ((v1.x * v2.x) +
			(v1.y * v2.y) +
			(v1.z * v2.z));
}

//============================================================================================================

inline Vector3f Normalize (const Vector3f& v)
{
#ifdef R5_USE_INVSQRT
	return v * Float::InvSqrt(v.x * v.x + v.y * v.y + v.z * v.z);
#else
	float mag = Float::Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return ( mag > FLOAT_TOLERANCE ? v / mag : Vector3f(0.0f, 0.0f, 1.0f) );
#endif
}

//============================================================================================================
// Convenience function that returns a normalized quaternion
//============================================================================================================

inline Quaternion Normalize (Quaternion q)
{
	q.Normalize();
	return q;
}

//============================================================================================================
// Normalizes a height map, bringing all values into relative 0-1 range
//============================================================================================================

void Normalize (float* data, uint allocated);

//============================================================================================================
// Projects a vector onto a normal vector
//============================================================================================================

inline Vector3f Project (const Vector3f& v, const Vector3f& normal)
{
	return normal * Dot(v, normal);
}

//============================================================================================================
// Flattens the vector onto a plane formed by the normal
//============================================================================================================

inline Vector3f Flatten (const Vector3f& v, const Vector3f& normal)
{
	return v - normal * Dot(v, normal);
}

//============================================================================================================
// Reflects the vector against the normal
//============================================================================================================

inline Vector3f Reflect (const Vector3f& v, const Vector3f& normal)
{
	return v - normal * (Dot(v, normal) * 2.0f);
}

//============================================================================================================
//    v0 (+y)       
//    +             
//    |\              The triangle's normal is towards you.
//    | \             Vertices must be provided counter-clockwise.
//    |  \
// v1 +---+ v2 (+x) 
//============================================================================================================

inline Vector3f GetNormal (const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
{
	return Normalize( Cross(v2 - v1, v0 - v1) );
}

//============================================================================================================
// Finds an angle between two vectors
//============================================================================================================

inline float GetAngle (const Vector3f& v0, const Vector3f& v1)
{										
	float mag = Magnitude(v0) * Magnitude(v1);
	return (mag > FLOAT_TOLERANCE) ? Float::Acos(Dot(v0, v1) / mag) : 0.0f;
}

//============================================================================================================
// Finds an angle between two quaternions
//============================================================================================================

inline float GetAngle (const Quaternion& q0, const Quaternion& q1)
{
	return Float::Acos( q0.GetForward().Dot(q1.GetForward()) );
}

//============================================================================================================
// Rotates a vertex around the given axis
//============================================================================================================

inline Vector3f RotateVertex (const Vector3f& vertex, const Vector3f& axis, float radAngle)
{
	Vector3f vec(vertex);
	vec.Rotate(axis, radAngle);
	return vec;
}

//============================================================================================================
// Find rotation angle and arbitrary axis that would rotate two vectors to match another set of vectors
//============================================================================================================

void GetAxisAngle (const Vector3f& vA1,
				   const Vector3f& vA2,
				   const Vector3f& vB1,
				   const Vector3f& vB2,
				   Vector3f& axis,
				   float &radAngle);
