#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Rotates the current vector around the axis by given degrees
//============================================================================================================

void Vector3f::Rotate (const Vector3f& axis, float radAngle)
{
	// Rotation is zero -- don't do anything
	if (Float::IsZero(radAngle)) return;

	// Pre-calculate the values to make things faster
	float fCos	= Float::Cos(radAngle),
		fSin	= Float::Sin(radAngle),
		fInvCos = 1 - fCos,
		fSinX	= fSin	  * axis.x,
		fSinY	= fSin	  * axis.y,
		fSinZ	= fSin	  * axis.z,
		fInvX	= fInvCos * axis.x,
		fInvY	= fInvCos * axis.y,
		fInvZ	= fInvCos * axis.z,
		fInvXX	= fInvX   * axis.x,
		fInvXY	= fInvX   * axis.y,
		fInvXZ	= fInvX   * axis.z,
		fInvYY	= fInvY   * axis.y,
		fInvYZ	= fInvY   * axis.z,
		fInvZZ	= fInvZ   * axis.z;

	Set(x * (fCos   + fInvXX) + y * (fInvXY - fSinZ ) + z * (fInvXZ + fSinY ),
		x * (fInvXY + fSinZ ) + y * (fCos   + fInvYY) + z * (fInvYZ - fSinX ),
		x * (fInvXZ - fSinY ) + y * (fInvYZ + fSinX ) + z * (fCos   + fInvZZ));
}

//============================================================================================================
// Vertex transform by quaternion
// 28 arithmetic operations
//============================================================================================================

void Vector3f::operator *= (const Quaternion& q)
{
	Vector3f uv  ( R5::Cross(q.xyz(), *this) );	// 9 operations
	Vector3f uuv ( R5::Cross(q.xyz(), uv) );	// 9 operations

	uv *= q.w;									// 3 operations

	(*this) += (uv + uuv) * 2.0f;				// 7 operations
}

//============================================================================================================

Vector3f Vector3f::operator * (const Quaternion& q) const
{
	Vector3f uv  ( R5::Cross(q.xyz(), *this) );
	Vector3f uuv ( R5::Cross(q.xyz(), uv) );

	uv *= q.w;

	return (*this) + (uv + uuv) * 2.0f;
}