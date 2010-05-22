#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Creates a 4x3 matrix from a quaternion
//============================================================================================================

Matrix43::Matrix43 (const Quaternion& q)
{
	mF[3]	= 0.0f;
	mF[7]	= 0.0f;
	mF[11]	= 0.0f;
	mF[12]	= 0.0f;
	mF[13]	= 0.0f;
	mF[14]	= 0.0f;
	mF[15]	= 1.0f;
	(*this) = q;
}

//============================================================================================================
// Set the rotation component by the specified quaternion
//============================================================================================================
// 30 arithmetic operations
//============================================================================================================
//	Matrix =  [ 1 - 2yy - 2zz		 2xy - 2zw       2xz + 2yw
//		            2xy + 2zw    1 - 2xx - 2zz		 2yz - 2xw
//			        2xz - 2yw	     2yz + 2xw   1 - 2xx - 2yy ]
//============================================================================================================

void Matrix43::operator =(const Quaternion& q)
{
	float xx, xy, xz, xw, yy, yz, yw, zz, zw;

	xx = q.x * q.x;
    xy = q.x * q.y;
    xz = q.x * q.z;
    xw = q.x * q.w;

    yy = q.y * q.y;
    yz = q.y * q.z;
    yw = q.y * q.w;

    zz = q.z * q.z;
    zw = q.z * q.w;

	mF[0]  = 1.0f - 2.0f * (yy + zz);
	mF[1]  =        2.0f * (xy + zw);
	mF[2]  =        2.0f * (xz - yw);

	mF[4]  =        2.0f * (xy - zw);
	mF[5]  = 1.0f - 2.0f * (xx + zz);
	mF[6]  =        2.0f * (yz + xw);

	mF[8]  =        2.0f * (xz + yw);
	mF[9]  =        2.0f * (yz - xw);
	mF[10] = 1.0f - 2.0f * (xx + yy);
}

//============================================================================================================
// Fast matrix * matrix multiplication for world transform matrices
// 63 arithmetic operations
//============================================================================================================

void Matrix43::operator *= (const Matrix43& mat)
{
	const float* f = mat.mF;
	float a, b, c;

	a  = f[0] * mF[0]  + f[4] * mF[1]  + f[8]  * mF[2];
	b  = f[1] * mF[0]  + f[5] * mF[1]  + f[9]  * mF[2];
	c  = f[2] * mF[0]  + f[6] * mF[1]  + f[10] * mF[2];

	mF[0] = a;
	mF[1] = b;
	mF[2] = c;

	a  = f[0] * mF[4]  + f[4] * mF[5]  + f[8]  * mF[6];
	b  = f[1] * mF[4]  + f[5] * mF[5]  + f[9]  * mF[6];
	c  = f[2] * mF[4]  + f[6] * mF[5]  + f[10] * mF[6];

	mF[4] = a;
	mF[5] = b;
	mF[6] = c;

	a  = f[0] * mF[8]  + f[4] * mF[9]  + f[8]  * mF[10];
	b  = f[1] * mF[8]  + f[5] * mF[9]  + f[9]  * mF[10];
	c  = f[2] * mF[8]  + f[6] * mF[9]  + f[10] * mF[10];

	mF[8]  = a;
	mF[9]  = b;
	mF[10] = c;

	a  = f[0] * mF[12] + f[4] * mF[13] + f[8]  * mF[14] + f[12];
	b  = f[1] * mF[12] + f[5] * mF[13] + f[9]  * mF[14] + f[13];
	c  = f[2] * mF[12] + f[6] * mF[13] + f[10] * mF[14] + f[14];

	mF[12] = a;
	mF[13] = b;
	mF[14] = c;
}

//============================================================================================================
// 4x3 matrix multiplication by a 4x3 matrix
// 63 arithmetic operations
//============================================================================================================

Matrix43 Matrix43::operator * (const Matrix43& mat) const
{
	Matrix43 out;
	const float* f = mat.mF;

	out[0]  = f[0] * mF[0]  + f[4] * mF[1]  + f[8]  * mF[2];
	out[1]  = f[1] * mF[0]  + f[5] * mF[1]  + f[9]  * mF[2];
	out[2]  = f[2] * mF[0]  + f[6] * mF[1]  + f[10] * mF[2];

	out[4]  = f[0] * mF[4]  + f[4] * mF[5]  + f[8]  * mF[6];
	out[5]  = f[1] * mF[4]  + f[5] * mF[5]  + f[9]  * mF[6];
	out[6]  = f[2] * mF[4]  + f[6] * mF[5]  + f[10] * mF[6];

	out[8]  = f[0] * mF[8]  + f[4] * mF[9]  + f[8]  * mF[10];
	out[9]  = f[1] * mF[8]  + f[5] * mF[9]  + f[9]  * mF[10];
	out[10] = f[2] * mF[8]  + f[6] * mF[9]  + f[10] * mF[10];

	out[12] = f[0] * mF[12] + f[4] * mF[13] + f[8]  * mF[14] + f[12];
	out[13] = f[1] * mF[12] + f[5] * mF[13] + f[9]  * mF[14] + f[13];
	out[14] = f[2] * mF[12] + f[6] * mF[13] + f[10] * mF[14] + f[14];

	return out;
}

//============================================================================================================
// 4x3 matrix multiplication by a 4x4 matrix
// 84 arithmetic operations
//============================================================================================================

Matrix44 Matrix43::operator * (const Matrix44& mat) const
{
	Matrix44 out;
	const float* f = mat.mF;

	out[0]  = f[0] * mF[0]  + f[4] * mF[1]  + f[8]  * mF[2];
	out[1]  = f[1] * mF[0]  + f[5] * mF[1]  + f[9]  * mF[2];
	out[2]  = f[2] * mF[0]  + f[6] * mF[1]  + f[10] * mF[2];
	out[3]  = f[3] * mF[0]  + f[7] * mF[1]  + f[11] * mF[2];

	out[4]  = f[0] * mF[4]  + f[4] * mF[5]  + f[8]  * mF[6];
	out[5]  = f[1] * mF[4]  + f[5] * mF[5]  + f[9]  * mF[6];
	out[6]  = f[2] * mF[4]  + f[6] * mF[5]  + f[10] * mF[6];
	out[7]  = f[3] * mF[4]  + f[7] * mF[5]  + f[11] * mF[6];

	out[8]  = f[0] * mF[8]  + f[4] * mF[9]  + f[8]  * mF[10];
	out[9]  = f[1] * mF[8]  + f[5] * mF[9]  + f[9]  * mF[10];
	out[10] = f[2] * mF[8]  + f[6] * mF[9]  + f[10] * mF[10];
	out[11] = f[3] * mF[8]  + f[7] * mF[9]  + f[11] * mF[10];

	out[12] = f[0] * mF[12] + f[4] * mF[13] + f[8]  * mF[14] + f[12];
	out[13] = f[1] * mF[12] + f[5] * mF[13] + f[9]  * mF[14] + f[13];
	out[14] = f[2] * mF[12] + f[6] * mF[13] + f[10] * mF[14] + f[14];
	out[15] = f[3] * mF[12] + f[7] * mF[13] + f[11] * mF[14] + f[15];

	return out;
}

//============================================================================================================
// ModelView matrix is calculated from eye position, lookAt direction, and up vectors
//============================================================================================================

void Matrix43::SetToView(	const Vector3f& eye,
							const Vector3f& dir,
							const Vector3f& up )
{	
	Vector3f s ( Cross(dir, up) );
	Vector3f u ( Cross(s, dir) );

	mF[ 0] =  s.x;
	mF[ 1] =  u.x;
	mF[ 2] = -dir.x;
	mF[ 3] =  0.0f;

	mF[ 4] =  s.y;
	mF[ 5] =  u.y;
	mF[ 6] = -dir.y;
	mF[ 7] =  0.0f;

	mF[ 8] =  s.z;
	mF[ 9] =  u.z;
	mF[10] = -dir.z;
	mF[11] =  0.0f;

	mF[12] = -mF[0] * eye.x - mF[4] * eye.y - mF[ 8] * eye.z;
	mF[13] = -mF[1] * eye.x - mF[5] * eye.y - mF[ 9] * eye.z;
	mF[14] = -mF[2] * eye.x - mF[6] * eye.y - mF[10] * eye.z;
	mF[15] = 1.0f;
}

//============================================================================================================
// Shortens the following:
//============================================================================================================
// SetIdentity();
// Translate(-1, 1);
// Scale(2/width, -2/height);
//============================================================================================================

void Matrix43::SetToScreen (float width, float height)
{
	mF[0]  =  2.0f / width;
	mF[1]  =  0.0f;
	mF[2]  =  0.0f;
	mF[3]  =  0.0f;

	mF[4]  =  0.0f;
	mF[5]  = -2.0f / height;
	mF[6]  =  0.0f;
	mF[7]  =  0.0f;

	mF[8]  =  0.0f;
	mF[9]  =  0.0f;
	mF[10] =  1.0f;
	mF[11] =  0.0f;

	mF[12] = -1.0f;
	mF[13] =  1.0f;
	mF[14] =  0.0f;
	mF[15] =  1.0f;
}

//============================================================================================================
// Sets the matrix to a centered orthographic projection
//============================================================================================================

void Matrix43::SetToOrtho (float width, float height, float near, float far)
{
	float depth	= far - near;

	mF[ 0] =  2.0f / width;
	mF[ 1] =  0.0f;
	mF[ 2] =  0.0f;
	mF[ 3] =  0.0f;

	mF[ 4] =  0.0f;
	mF[ 5] = -2.0f / height;
	mF[ 6] =  0.0f;
	mF[ 7] =  0.0f;

	mF[ 8] =  0.0f;
	mF[ 9] =  0.0f;
	mF[10] = -2.0f / depth;
	mF[11] =  0.0f;

	mF[12] = 0.0f;
	mF[13] = 0.0f;
	mF[14] = -(far + near) / depth;
	mF[15] =  1.0f;
}

//============================================================================================================
// Sets the matrix to a full orthographic projection
//============================================================================================================

void Matrix43::SetToOrtho (float minX, float minY, float maxX, float maxY, float near, float far)
{
	float width  = maxX - minX;
	float height = maxY - minY;
	float depth	 = far  - near;

	mF[ 0] =  2.0f / width;
	mF[ 1] =  0.0f;
	mF[ 2] =  0.0f;
	mF[ 3] =  0.0f;

	mF[ 4] =  0.0f;
	mF[ 5] = -2.0f / height;
	mF[ 6] =  0.0f;
	mF[ 7] =  0.0f;

	mF[ 8] =  0.0f;
	mF[ 9] =  0.0f;
	mF[10] = -2.0f / depth;
	mF[11] =  0.0f;

	mF[12] = -(minX + maxX) / width;
	mF[13] =  (minY + maxY) / height;
	mF[14] = -(far  + near) / depth;
	mF[15] =  1.0f;
}

//============================================================================================================
// Optimized position+scaling transformation matrix -- just 16 assignment ops!
//============================================================================================================

void Matrix43::SetToTransform (const Vector3f& pos, float scale)
{
	mF[0] = scale;
	mF[1] = 0.0f;
	mF[2] = 0.0f;
	mF[3] = 0.0f;

	mF[4] = 0.0f;
	mF[5] = scale;
	mF[6] = 0.0f;
	mF[7] = 0.0f;

	mF[8]  = 0.0f;
	mF[9]  = 0.0f;
	mF[10] = scale;
	mF[11] = 0.0f;

	mF[12] = pos.x;
	mF[13] = pos.y;
	mF[14] = pos.z;
	mF[15] = 1.0f;
}

//============================================================================================================
// Optimized transformation matrix calculation. Performs (*this = rot), Scale and Translate
//============================================================================================================
// Non-1.0 scaling:  39 arithmetic, 1 comparison
// Scaling is 1.0:   30 arithmetic, 1 comparison
//============================================================================================================

void Matrix43::SetToTransform ( const Vector3f&		pos,
								const Quaternion&	rot,
								float				scale )
{
	float xx, xy, xz, xw, yy, yz, yw, zz, zw;

	xx = rot.x * rot.x;
    xy = rot.x * rot.y;
    xz = rot.x * rot.z;
    xw = rot.x * rot.w;

    yy = rot.y * rot.y;
    yz = rot.y * rot.z;
    yw = rot.y * rot.w;

    zz = rot.z * rot.z;
    zw = rot.z * rot.w;

	mF[0]  = 1.0f - 2.0f * (yy + zz);
	mF[1]  =        2.0f * (xy + zw);
	mF[2]  =        2.0f * (xz - yw);
	mF[3]  = 0.0f;

	mF[4]  =        2.0f * (xy - zw);
	mF[5]  = 1.0f - 2.0f * (xx + zz);
	mF[6]  =        2.0f * (yz + xw);
	mF[7]  = 0.0f;

	mF[8]  =        2.0f * (xz + yw);
	mF[9]  =        2.0f * (yz - xw);
	mF[10] = 1.0f - 2.0f * (xx + yy);
	mF[11] = 0.0f;

	mF[12] = pos.x;
	mF[13] = pos.y;
	mF[14] = pos.z;
	mF[15] = 1.0f;

	if (scale != 1.0f)
	{
		mF[0]  *= scale;
		mF[1]  *= scale;
		mF[2]  *= scale;
		mF[4]  *= scale;
		mF[5]  *= scale;
		mF[6]  *= scale;
		mF[8]  *= scale;
		mF[9]  *= scale;
		mF[10] *= scale;
	}
}

//============================================================================================================
// Optimized matrix setting function meant for bone transforms
// 76 arithmetic operations
//============================================================================================================
// This function performs the inlined equivalent of:
//		SetToIdentity();
//		Translate(invBindPos);
//		*this *= (absoluteRot * invBindRot);
//		Translate(absolutePos * scale);
//============================================================================================================

void Matrix43::SetToTransform (	const Vector3f&		invBindPos,
								const Vector3f&		absolutePos,
								const Quaternion&	invBindRot,
								const Quaternion&	absoluteRot )
{
	float fx, fy, fz, fw, xx, xy, xz, xw, yy, yz, yw, zz, zw;

	// Quaternion multiplication (absoluteRot * invBindRot)
	fx = absoluteRot.w * invBindRot.x +
		 absoluteRot.x * invBindRot.w +
		 absoluteRot.y * invBindRot.z -
		 absoluteRot.z * invBindRot.y;

	fy = absoluteRot.w * invBindRot.y -
		 absoluteRot.x * invBindRot.z +
		 absoluteRot.y * invBindRot.w +
		 absoluteRot.z * invBindRot.x;

	fz = absoluteRot.w * invBindRot.z +
		 absoluteRot.x * invBindRot.y -
		 absoluteRot.y * invBindRot.x +
		 absoluteRot.z * invBindRot.w;

	fw = absoluteRot.w * invBindRot.w -
		 absoluteRot.x * invBindRot.x -
		 absoluteRot.y * invBindRot.y -
		 absoluteRot.z * invBindRot.z;

	// Rotation (*this = rot)
	xx = fx * fx;
    xy = fx * fy;
    xz = fx * fz;
    xw = fx * fw;

    yy = fy * fy;
    yz = fy * fz;
    yw = fy * fw;

    zz = fz * fz;
    zw = fz * fw;

	mF[0]  = 1.0f - 2.0f * (yy + zz);
	mF[1]  =        2.0f * (xy + zw);
	mF[2]  =        2.0f * (xz - yw);
	mF[3]	= 0.0f;

	mF[4]  =        2.0f * (xy - zw);
	mF[5]  = 1.0f - 2.0f * (xx + zz);
	mF[6]  =        2.0f * (yz + xw);
	mF[7]	= 0.0f;

	mF[8]  =        2.0f * (xz + yw);
	mF[9]  =        2.0f * (yz - xw);
	mF[10] = 1.0f - 2.0f * (xx + yy);
	mF[11] = 0.0f;

	// Translation
	mF[12] = mF[0] * invBindPos.x + mF[4] * invBindPos.y + mF[8 ] * invBindPos.z + absolutePos.x;
	mF[13] = mF[1] * invBindPos.x + mF[5] * invBindPos.y + mF[9 ] * invBindPos.z + absolutePos.y;
	mF[14] = mF[2] * invBindPos.x + mF[6] * invBindPos.y + mF[10] * invBindPos.z + absolutePos.z;
	mF[15] = 1.0f;
}

//============================================================================================================
// Whether the matrix is an identity matrix
//============================================================================================================

bool Matrix43::IsIdentity()
{
	return (mF[0]  == 1.0f &&
			mF[5]  == 1.0f &&
			mF[10] == 1.0f &&
			mF[12] == 0.0f &&
			mF[13] == 0.0f &&
			mF[14] == 0.0f);
}

//============================================================================================================
// Resets the given matrix array to an identity matrix
//============================================================================================================

void Matrix43::SetToIdentity()
{
	memset(mF, 0, 64);
	mF[0]  = 1.0f;
	mF[5]  = 1.0f;
	mF[10] = 1.0f;
	mF[15] = 1.0f;
}

//============================================================================================================
// Rotates the matrix around the given axis by the given angle
//============================================================================================================

void Matrix43::Rotate (const Vector3f& axis, float radAngle)
{
	if ( Float::IsZero(radAngle) ) return;

	// Pre-calculate the values to make things faster
	float fCos	= Float::Cos(radAngle),
		fSin	= Float::Sin(radAngle),
		fInvCos = 1 - fCos,
		fSinX	= axis.x * fSin,
		fSinY	= axis.y * fSin,
		fSinZ	= axis.z * fSin,
		fXX		= axis.x * axis.x,
		fXY		= axis.x * axis.y,
		fXZ		= axis.x * axis.z,
		fYY		= axis.y * axis.y,
		fYZ		= axis.y * axis.z,
		fZZ		= axis.z * axis.z;

	// Create a rotation matrix from the axis and angle provided
	float f[9], a, b, c;

	f[0] = fXX * fInvCos + fCos;
	f[1] = fXY * fInvCos + fSinZ;
	f[2] = fXZ * fInvCos - fSinY;
	f[3] = fXY * fInvCos - fSinZ;
	f[4] = fYY * fInvCos + fCos;
	f[5] = fYZ * fInvCos + fSinX;
	f[6] = fXZ * fInvCos + fSinY;
	f[7] = fYZ * fInvCos - fSinX;
	f[8] = fZZ * fInvCos + fCos;

	a = f[0] * mF[0]  + f[3] * mF[1]  + f[6] * mF[2];
	b = f[1] * mF[0]  + f[4] * mF[1]  + f[7] * mF[2];
	c = f[2] * mF[0]  + f[5] * mF[1]  + f[8] * mF[2];

	mF[0] = a;
	mF[1] = b;
	mF[2] = c;

	a = f[0] * mF[4]  + f[3] * mF[5]  + f[6] * mF[6];
	b = f[1] * mF[4]  + f[4] * mF[5]  + f[7] * mF[6];
	c = f[2] * mF[4]  + f[5] * mF[5]  + f[8] * mF[6];

	mF[4] = a;
	mF[5] = b;
	mF[6] = c;

	a = f[0] * mF[8]  + f[3] * mF[9]  + f[6] * mF[10];
	b = f[1] * mF[8]  + f[4] * mF[9]  + f[7] * mF[10];
	c = f[2] * mF[8]  + f[5] * mF[9]  + f[8] * mF[10];

	mF[8]  = a;
	mF[9]  = b;
	mF[10] = c;

	a = f[0] * mF[12] + f[3] * mF[13] + f[6] * mF[14];
	b = f[1] * mF[12] + f[4] * mF[13] + f[7] * mF[14];
	c = f[2] * mF[12] + f[5] * mF[13] + f[8] * mF[14];

	mF[12] = a;
	mF[13] = b;
	mF[14] = c;
}
//============================================================================================================
// Scales the matrix
// 12 arithmetic operations
//============================================================================================================

void Matrix43::Scale (float scale)
{
	mF[0]  *= scale;
	mF[1]  *= scale;
	mF[2]  *= scale;
	mF[4]  *= scale;
	mF[5]  *= scale;
	mF[6]  *= scale;
	mF[8]  *= scale;
	mF[9]  *= scale;
	mF[10] *= scale;
	mF[12] *= scale;
	mF[13] *= scale;
	mF[14] *= scale;
}

//============================================================================================================

void Matrix43::Scale (const Vector3f& scale)
{
	mF[0]  *= scale.x;
	mF[1]  *= scale.y;
	mF[2]  *= scale.z;
	mF[4]  *= scale.x;
	mF[5]  *= scale.y;
	mF[6]  *= scale.z;
	mF[8]  *= scale.x;
	mF[9]  *= scale.y;
	mF[10] *= scale.z;
	mF[12] *= scale.x;
	mF[13] *= scale.y;
	mF[14] *= scale.z;
}

//============================================================================================================
// Quick inversion of a matrix -- from www.humus.name
// 69 arithmetic operations + memcpy
//============================================================================================================

void Matrix43::Invert()
{
	float out[16], det;

	det  = mF[0] * mF[5] * mF[10];
	det += mF[4] * mF[9] * mF[2];
	det += mF[8] * mF[1] * mF[6];
	det -= mF[8] * mF[5] * mF[2];
	det -= mF[4] * mF[1] * mF[10];
	det -= mF[0] * mF[9] * mF[6];
	det  = 1.0f / det;

	out[ 0] =  (mF[5] * mF[10] - mF[9] * mF[6]) * det;
	out[ 1] = -(mF[1] * mF[10] - mF[9] * mF[2]) * det;
	out[ 2] =  (mF[1] * mF[ 6] - mF[5] * mF[2]) * det;
	out[ 3] = 0.0f;

	out[ 4] = -(mF[4] * mF[10] - mF[8] * mF[6]) * det;
	out[ 5] =  (mF[0] * mF[10] - mF[8] * mF[2]) * det;
	out[ 6] = -(mF[0] * mF[ 6] - mF[4] * mF[2]) * det;
	out[ 7] = 0.0f;

	out[ 8] =  (mF[4] * mF[9] - mF[8] * mF[5]) * det;
	out[ 9] = -(mF[0] * mF[9] - mF[8] * mF[1]) * det;
	out[10] =  (mF[0] * mF[5] - mF[4] * mF[1]) * det;
	out[11] = 0.0f;

	out[12] = -(mF[12] * out[0] + mF[13] * out[4] + mF[14] * out[8]);
	out[13] = -(mF[12] * out[1] + mF[13] * out[5] + mF[14] * out[9]);
	out[14] = -(mF[12] * out[2] + mF[13] * out[6] + mF[14] * out[10]);
	out[15] = 1.0f;

	memcpy(mF, out, 64);
}

//============================================================================================================
// Transposes the 3x3 rotation part of the matrix
//============================================================================================================

void Matrix43::Transpose()
{
	float temp[6];

	temp[0] = mF[4];
	temp[1] = mF[8];
	temp[2] = mF[1];
	temp[3] = mF[9];
	temp[4] = mF[2];
	temp[5] = mF[6];

	mF[1] = temp[0];
	mF[2] = temp[1];
	mF[4] = temp[2];
	mF[6] = temp[3];
	mF[8] = temp[4];
	mF[9] = temp[5];
}

//============================================================================================================
// Eliminates the rotational and scaling components of the matrix, replacing it with the specified scaling
//============================================================================================================

void Matrix43::ReplaceScaling (float scale)
{
	mF[ 0] = scale;
	mF[ 1] = 0.0f;
	mF[ 2] = 0.0f;
	mF[ 4] = 0.0f;
	mF[ 5] = scale;
	mF[ 6] = 0.0f;
	mF[ 8] = 0.0f;
	mF[ 9] = 0.0f;
	mF[10] = scale;
}

//============================================================================================================
// Translate in Matrix space
// 18 arithmetic operations
//============================================================================================================

void Matrix43::PreTranslate(const Vector3f& v)
{
	x += mF[0] * v.x + mF[4] * v.y + mF[8 ] * v.z;
	y += mF[1] * v.x + mF[5] * v.y + mF[9 ] * v.z;
	z += mF[2] * v.x + mF[6] * v.y + mF[10] * v.z;
}

//============================================================================================================
// Equivalent of: *this = Matrix43(scale) * (*this)
// 9 arithmetic operations
//============================================================================================================

void Matrix43::PreScale (float scale)
{
	mF[0]  *= scale;
	mF[1]  *= scale;
	mF[2]  *= scale;
	mF[4]  *= scale;
	mF[5]  *= scale;
	mF[6]  *= scale;
	mF[8]  *= scale;
	mF[9]  *= scale;
	mF[10] *= scale;
}

//============================================================================================================

void Matrix43::PreScale (const Vector3f& scale)
{
	mF[0]  *= scale.x;
	mF[1]  *= scale.x;
	mF[2]  *= scale.x;
	mF[4]  *= scale.y;
	mF[5]  *= scale.y;
	mF[6]  *= scale.y;
	mF[8]  *= scale.z;
	mF[9]  *= scale.z;
	mF[10] *= scale.z;
}

//============================================================================================================
// Equivalent of: *this = Matrix43(rot) * (*this)
// 75 arithmetic operations
//============================================================================================================

void Matrix43::PreRotate (const Quaternion& rot)
{
	Matrix43 out;
	float xx, xy, xz, xw, yy, yz, yw, zz, zw, a, b, c;

	xx = rot.x * rot.x;
	xy = rot.x * rot.y;
	xz = rot.x * rot.z;
	xw = rot.x * rot.w;
	yy = rot.y * rot.y;
	yz = rot.y * rot.z;
	yw = rot.y * rot.w;
	zz = rot.z * rot.z;
	zw = rot.z * rot.w;

	a = 1.0f - 2.0f * (yy + zz);
	b =        2.0f * (xy + zw);
	c =        2.0f * (xz - yw);

	out[0]  = mF[0] * a  + mF[4] * b  + mF[8]  * c;
	out[1]  = mF[1] * a  + mF[5] * b  + mF[9]  * c;
	out[2]  = mF[2] * a  + mF[6] * b  + mF[10] * c;

	a =        2.0f * (xy - zw);
	b = 1.0f - 2.0f * (xx + zz);
	c =        2.0f * (yz + xw);

	out[4]  = mF[0] * a  + mF[4] * b  + mF[8]  * c;
	out[5]  = mF[1] * a  + mF[5] * b  + mF[9]  * c;
	out[6]  = mF[2] * a  + mF[6] * b  + mF[10] * c;

	a =        2.0f * (xz + yw);
	b =        2.0f * (yz - xw);
	c = 1.0f - 2.0f * (xx + yy);

	out[8]  = mF[0] * a  + mF[4] * b  + mF[8]  * c;
	out[9]  = mF[1] * a  + mF[5] * b  + mF[9]  * c;
	out[10] = mF[2] * a  + mF[6] * b  + mF[10] * c;

	mF[0] = out[0];
	mF[1] = out[1];
	mF[2] = out[2];

	mF[4] = out[4];
	mF[5] = out[5];
	mF[6] = out[6];

	mF[ 8] = out[ 8];
	mF[ 9] = out[ 9];
	mF[10] = out[10];
}