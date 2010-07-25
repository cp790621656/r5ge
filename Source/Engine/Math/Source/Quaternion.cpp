#include "../Include/_All.h"

using namespace R5;

//============================================================================================================
// Retrieves the rotational component from the matrix
//============================================================================================================

void Quaternion::operator =(const Matrix43& mIn)
{
	const float *m = mIn.mF;
	float trace = m[0] + m[5] + m[10] + 1;

	if (trace > FLOAT_TOLERANCE)
	{
		float s = 0.5f / Float::Sqrt(trace);
		w = 0.25f / s;
		x = (m[6] - m[9]) * s;
		y = (m[8] - m[2]) * s;
		z = (m[1] - m[4]) * s;
	}
	else
	{
		if (m[0] > m[5] && m[0] > m[10])
		{
			float s = Float::InvSqrt(1.0f + m[0] - m[5] - m[10]);
			x = 0.5f / s;
			s = 0.5f * s;
			y = (m[4] + m[1]) * s;
			z = (m[8] + m[2]) * s;
			w = (m[9] - m[6]) * s;
	
		}
		else if (m[5] > m[10])
		{
			float s = Float::InvSqrt(1.0f + m[5] - m[0] - m[10]);
			y = 0.5f / s;
			s = 0.5f * s;
			x = (m[4] + m[1]) * s;
			z = (m[9] + m[6]) * s;
			w = (m[8] - m[2]) * s;
		}
		else
		{
			float s = Float::InvSqrt(1.0f + m[10] - m[0] - m[5]);
			z = 0.5f / s;
			s = 0.5f * s;
			x = (m[8] + m[2]) * s;
			y = (m[9] + m[6]) * s;
			w = (m[4] - m[1]) * s;
		}
	}

	Normalize();
}

//============================================================================================================
// Sets the quaternion from Pitch (X), Roll (Y), and Yaw (Z) rotations specified in radians
//============================================================================================================

void Quaternion::SetFromEuler (const Vector3f& rad)
{
	float halfX = rad.x * 0.5f;
	float halfY = rad.y * 0.5f;
	float halfZ = rad.z * 0.5f;

	float sinx = Float::Sin(halfX);
	float siny = Float::Sin(halfY);
	float sinz = Float::Sin(halfZ);

	float cosx = Float::Cos(halfX);
	float cosy = Float::Cos(halfY);
	float cosz = Float::Cos(halfZ);

	float xx = siny * sinz;
	float yy = siny * cosz;
	float zz = cosy * sinz;
	float ww = cosy * cosz;

	x = ww * sinx + xx * cosx;
	y = zz * sinx + yy * cosx;
	z = zz * cosx - yy * sinx;
	w = ww * cosx - xx * sinx;

	Normalize();
}

//============================================================================================================
// Sets the quaternion's coordinates based on the given axis of rotation and angle combo
//============================================================================================================

void Quaternion::SetFromAxisAngle(const Vector3f& axis, float radAngle)
{
	float sa = Float::Sin(radAngle *= 0.5f);
	x = axis.x * sa;
	y = axis.y * sa;
	z = axis.z * sa;
	w = Float::Cos(radAngle);
}

//============================================================================================================
// Sets the quaternion's coordinates based on the given vector of direction
// NOTE: This method assumes that there is no "roll" movement
//============================================================================================================

void Quaternion::SetFromDirection (const Vector3f& direction)
{
	Set(0.0f, 0.0f, 0.0f, 1.0f);
    Vector3f dir(direction);
    dir.Normalize();

    Vector2f horizontal(dir.x, dir.y);
    horizontal.Normalize();

	float horizAngle = 0.5f * Float::Acos(horizontal.y);
	float vertAngle  = 0;
	
	float dotProduct = dir.x * horizontal.x + dir.y * horizontal.y;
	if (dotProduct < 1.0f) vertAngle = 0.5f * Float::Acos(dotProduct);

    if (vertAngle > FLOAT_TOLERANCE)
	{
		if (dir.z < 0) vertAngle = -vertAngle;
		float sa = Float::Sin(vertAngle);

		x =  horizontal.y * sa;
		y = -horizontal.x * sa;
		w =  Float::Cos(vertAngle);

		if (horizAngle > FLOAT_TOLERANCE)
		{
			if (dir.x < 0) horizAngle = -horizAngle;
			*this *= Quaternion ( 0, 0, -Float::Sin(horizAngle), Float::Cos(horizAngle) );
		}
	}
	else if (horizAngle > FLOAT_TOLERANCE)
    {
		if (dir.x < 0) horizAngle = -horizAngle;

		z = -Float::Sin(horizAngle);
		w =  Float::Cos(horizAngle);
	}

	Normalize();
}

//============================================================================================================
// Conversion to euler angles (in radians)
//============================================================================================================

Vector3f Quaternion::GetEuler() const
{
	float xx	= x * x;
    float yy	= y * y;
    float zz	= z * z;
	float ww	= w * w;
	float unit	= xx + yy + zz + ww;
	float half	= unit * 0.499f;
	float test	= x * y + z * w;

	if (test >  half) return Vector3f(0.0f,  2.0f * Float::Atan2(x, w),  HALFPI);
	if (test < -half) return Vector3f(0.0f, -2.0f * Float::Atan2(x, w), -HALFPI);

	return Vector3f(Float::Atan2(2.0f * x * w - 2.0f * y * z , -xx + yy - zz + ww),
					Float::Atan2(2.0f * y * w - 2.0f * x * z ,  xx - yy - zz + ww),
					Float::Asin (2.0f * test / unit));
}

//============================================================================================================
// Conversion to axis-angle (angle is in radians)
//============================================================================================================

void Quaternion::GetAxisAngle(Vector3f& axis, float& angle) const
{
	angle = 2.0f * Float::Acos(w);
	axis.Set(x, y, z);
	axis *= Float::InvSqrt(1.0f - w * w);
}