#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Projection matrix is calculated from field of view, aspect ratio, and near/far clipping planes
//============================================================================================================

void Matrix44::SetToProjection( float fov,
								float aspect,
								float near,
								float far )
{
	float f ( 1.0f / TAN( DEG2RAD(fov * 0.25f) ) );
	float n ( near - far );

	mF[ 0] =  f / aspect;
	mF[ 1] =  0.0f;
	mF[ 2] =  0.0f;
	mF[ 3] =  0.0f;

	mF[ 4] =  0.0f;
	mF[ 5] =  f;
	mF[ 6] =  0.0f;
	mF[ 7] =  0.0f;

	mF[ 8] =  0.0f;
	mF[ 9] =  0.0f;
	mF[10] = (far + near) / n;
	mF[11] = -1.0f;

	mF[12] =  0.0f;
	mF[13] =  0.0f;
	mF[14] = (2.0f * far * near) / n;
	mF[15] =  0.0f;
}

//============================================================================================================
// 4x4 matrix multiplication by a 4x3 matrix
// 84 arithmetic operations
//============================================================================================================

Matrix44 Matrix44::operator * (const Matrix43& mat) const
{
	Matrix44 out;
	const float* f = mat.mF;

	out[0]  = f[0] * mF[0]  + f[4] * mF[1]  + f[8]  * mF[2]  + f[12] * mF[3];
	out[1]  = f[1] * mF[0]  + f[5] * mF[1]  + f[9]  * mF[2]  + f[13] * mF[3];
	out[2]  = f[2] * mF[0]  + f[6] * mF[1]  + f[10] * mF[2]  + f[14] * mF[3];
	out[3]  = mF[3];

	out[4]  = f[0] * mF[4]  + f[4] * mF[5]  + f[8]  * mF[6]  + f[12] * mF[7];
	out[5]  = f[1] * mF[4]  + f[5] * mF[5]  + f[9]  * mF[6]  + f[13] * mF[7];
	out[6]  = f[2] * mF[4]  + f[6] * mF[5]  + f[10] * mF[6]  + f[14] * mF[7];
	out[7]  = mF[7];

	out[8]  = f[0] * mF[8]  + f[4] * mF[9]  + f[8]  * mF[10] + f[12] * mF[11];
	out[9]  = f[1] * mF[8]  + f[5] * mF[9]  + f[9]  * mF[10] + f[13] * mF[11];
	out[10] = f[2] * mF[8]  + f[6] * mF[9]  + f[10] * mF[10] + f[14] * mF[11];
	out[11] = mF[11];

	out[12] = f[0] * mF[12] + f[4] * mF[13] + f[8]  * mF[14] + f[12] * mF[15];
	out[13] = f[1] * mF[12] + f[5] * mF[13] + f[9]  * mF[14] + f[13] * mF[15];
	out[14] = f[2] * mF[12] + f[6] * mF[13] + f[10] * mF[14] + f[14] * mF[15];
	out[15] = mF[15];

	return out;
}

//============================================================================================================
// Multiplies the existing matrix by the provided one
// 84 arithmetic operations
//============================================================================================================

void Matrix44::operator *= (const Matrix43& mat)
{
	const float* f = mat.mF;
	float a, b, c;

	a = f[0] * mF[0]  + f[4] * mF[1]  + f[8]  * mF[2]  + f[12] * mF[3];
	b = f[1] * mF[0]  + f[5] * mF[1]  + f[9]  * mF[2]  + f[13] * mF[3];
	c = f[2] * mF[0]  + f[6] * mF[1]  + f[10] * mF[2]  + f[14] * mF[3];

	mF[0] = a;
	mF[1] = b;
	mF[2] = c;

	a = f[0] * mF[4]  + f[4] * mF[5]  + f[8]  * mF[6]  + f[12] * mF[7];
	b = f[1] * mF[4]  + f[5] * mF[5]  + f[9]  * mF[6]  + f[13] * mF[7];
	c = f[2] * mF[4]  + f[6] * mF[5]  + f[10] * mF[6]  + f[14] * mF[7];

	mF[4] = a;
	mF[5] = b;
	mF[6] = c;

	a = f[0] * mF[8]  + f[4] * mF[9]  + f[8]  * mF[10] + f[12] * mF[11];
	b = f[1] * mF[8]  + f[5] * mF[9]  + f[9]  * mF[10] + f[13] * mF[11];
	c = f[2] * mF[8]  + f[6] * mF[9]  + f[10] * mF[10] + f[14] * mF[11];

	mF[8]  = a;
	mF[9]  = b;
	mF[10] = c;

	a = f[0] * mF[12] + f[4] * mF[13] + f[8]  * mF[14] + f[12] * mF[15];
	b = f[1] * mF[12] + f[5] * mF[13] + f[9]  * mF[14] + f[13] * mF[15];
	c = f[2] * mF[12] + f[6] * mF[13] + f[10] * mF[14] + f[14] * mF[15];

	mF[12] = a;
	mF[13] = b;
	mF[14] = c;
}

//============================================================================================================
// Multiplies the existing matrix by the provided one
// 112 arithmetic operations
//============================================================================================================

void Matrix44::operator *= (const Matrix44& mat)
{
	const float* f = mat.mF;
	float a, b, c, d;

	a = f[0] * mF[0]  + f[4] * mF[1]  + f[8]  * mF[2]  + f[12] * mF[3];
	b = f[1] * mF[0]  + f[5] * mF[1]  + f[9]  * mF[2]  + f[13] * mF[3];
	c = f[2] * mF[0]  + f[6] * mF[1]  + f[10] * mF[2]  + f[14] * mF[3];
	d = f[3] * mF[0]  + f[7] * mF[1]  + f[11] * mF[2]  + f[15] * mF[3];

	mF[0] = a;
	mF[1] = b;
	mF[2] = c;
	mF[3] = d;

	a = f[0] * mF[4]  + f[4] * mF[5]  + f[8]  * mF[6]  + f[12] * mF[7];
	b = f[1] * mF[4]  + f[5] * mF[5]  + f[9]  * mF[6]  + f[13] * mF[7];
	c = f[2] * mF[4]  + f[6] * mF[5]  + f[10] * mF[6]  + f[14] * mF[7];
	d = f[3] * mF[4]  + f[7] * mF[5]  + f[11] * mF[6]  + f[15] * mF[7];

	mF[4] = a;
	mF[5] = b;
	mF[6] = c;
	mF[7] = d;

	a = f[0] * mF[8]  + f[4] * mF[9]  + f[8]  * mF[10] + f[12] * mF[11];
	b = f[1] * mF[8]  + f[5] * mF[9]  + f[9]  * mF[10] + f[13] * mF[11];
	c = f[2] * mF[8]  + f[6] * mF[9]  + f[10] * mF[10] + f[14] * mF[11];
	d = f[3] * mF[8]  + f[7] * mF[9]  + f[11] * mF[10] + f[15] * mF[11];

	mF[8]  = a;
	mF[9]  = b;
	mF[10] = c;
	mF[11] = d;

	a = f[0] * mF[12] + f[4] * mF[13] + f[8]  * mF[14] + f[12] * mF[15];
	b = f[1] * mF[12] + f[5] * mF[13] + f[9]  * mF[14] + f[13] * mF[15];
	c = f[2] * mF[12] + f[6] * mF[13] + f[10] * mF[14] + f[14] * mF[15];
	d = f[3] * mF[12] + f[7] * mF[13] + f[11] * mF[14] + f[15] * mF[15];

	mF[12] = a;
	mF[13] = b;
	mF[14] = c;
	mF[15] = d;
}

//============================================================================================================
// Gets a determinant of the matrix for inversion
// 96 arithmetic operations
//============================================================================================================

float GetDeterminant (const float mat[16])
{
  return	mat[12] * mat[9]  * mat[6]  * mat[3] -
			mat[8]  * mat[13] * mat[6]  * mat[3] -
			mat[12] * mat[5]  * mat[10] * mat[3] +
			mat[4]  * mat[13] * mat[10] * mat[3] +
			mat[8]  * mat[5]  * mat[14] * mat[3] -
			mat[4]  * mat[9]  * mat[14] * mat[3] -
			mat[12] * mat[9]  * mat[2]  * mat[7] +
			mat[8]  * mat[13] * mat[2]  * mat[7] +
			mat[12] * mat[1]  * mat[10] * mat[7] -
			mat[0]  * mat[13] * mat[10] * mat[7] -
			mat[8]  * mat[1]  * mat[14] * mat[7] +
			mat[0]  * mat[9]  * mat[14] * mat[7] +
			mat[12] * mat[5]  * mat[2]  * mat[11] -
			mat[4]  * mat[13] * mat[2]  * mat[11] -
			mat[12] * mat[1]  * mat[6]  * mat[11] +
			mat[0]  * mat[13] * mat[6]  * mat[11] +
			mat[4]  * mat[1]  * mat[14] * mat[11] -
			mat[0]  * mat[5]  * mat[14] * mat[11] -
			mat[8]  * mat[5]  * mat[2]  * mat[15] +
			mat[4]  * mat[9]  * mat[2]  * mat[15] +
			mat[8]  * mat[1]  * mat[6]  * mat[15] -
			mat[0]  * mat[9]  * mat[6]  * mat[15] -
			mat[4]  * mat[1]  * mat[10] * mat[15] +
			mat[0]  * mat[5]  * mat[10] * mat[15];
}

//============================================================================================================
// Inverses a matrix
//============================================================================================================
// 386 arithmetic operations, 1 memcopy (18 assignments)
//============================================================================================================

void Matrix44::Invert()
{
	float fDet = GetDeterminant(mF), out[16];
    if (fDet == 0.0f) return;
    fDet = 1.0f / fDet;

    out[0]  =  (-mF[13] * mF[10] * mF[7 ] + mF[9 ] * mF[14] * mF[7 ] + mF[13] * mF[6 ] * mF[11] -
				 mF[5 ] * mF[14] * mF[11] - mF[9 ] * mF[6 ] * mF[15] + mF[5 ] * mF[10] * mF[15]) * fDet;
	out[1]  =  ( mF[13] * mF[10] * mF[3 ] - mF[9 ] * mF[14] * mF[3 ] - mF[13] * mF[2 ] * mF[11] +
				 mF[1 ] * mF[14] * mF[11] + mF[9 ] * mF[2 ] * mF[15] - mF[1 ] * mF[10] * mF[15]) * fDet;
	out[2]  =  (-mF[13] * mF[6 ] * mF[3 ] + mF[5 ] * mF[14] * mF[3 ] + mF[13] * mF[2 ] * mF[7 ] -
				 mF[1 ] * mF[14] * mF[7 ] - mF[5 ] * mF[2 ] * mF[15] + mF[1 ] * mF[6 ] * mF[15]) * fDet;
	out[3]  =  ( mF[9 ] * mF[6 ] * mF[3 ] - mF[5 ] * mF[10] * mF[3 ] - mF[9 ] * mF[2 ] * mF[7 ] +
				 mF[1 ] * mF[10] * mF[7 ] + mF[5 ] * mF[2 ] * mF[11] - mF[1 ] * mF[6 ] * mF[11]) * fDet;

    out[4]  =  ( mF[12] * mF[10] * mF[7 ] - mF[8 ] * mF[14] * mF[7 ] - mF[12] * mF[6 ] * mF[11] +
				 mF[4 ] * mF[14] * mF[11] + mF[8 ] * mF[6 ] * mF[15] - mF[4 ] * mF[10] * mF[15]) * fDet;
	out[5]  =  (-mF[12] * mF[10] * mF[3 ] + mF[8 ] * mF[14] * mF[3 ] + mF[12] * mF[2 ] * mF[11] -
				 mF[0 ] * mF[14] * mF[11] - mF[8 ] * mF[2 ] * mF[15] + mF[0 ] * mF[10] * mF[15]) * fDet;
	out[6]  =  ( mF[12] * mF[6 ] * mF[3 ] - mF[4 ] * mF[14] * mF[3 ] - mF[12] * mF[2 ] * mF[7 ] +
				 mF[0 ] * mF[14] * mF[7 ] + mF[4 ] * mF[2 ] * mF[15] - mF[0 ] * mF[6 ] * mF[15]) * fDet;
	out[7]  =  (-mF[8 ] * mF[6 ] * mF[3 ] + mF[4 ] * mF[10] * mF[3 ] + mF[8 ] * mF[2 ] * mF[7 ] -
				 mF[0 ] * mF[10] * mF[7 ] - mF[4 ] * mF[2 ] * mF[11] + mF[0 ] * mF[6 ] * mF[11]) * fDet;

    out[8]  =  (-mF[12] * mF[9 ] * mF[7 ] + mF[8 ] * mF[13] * mF[7 ] + mF[12] * mF[5 ] * mF[11] -
				 mF[4 ] * mF[13] * mF[11] - mF[8 ] * mF[5 ] * mF[15] + mF[4 ] * mF[9 ] * mF[15]) * fDet;
	out[9]  =  ( mF[12] * mF[9 ] * mF[3 ] - mF[8 ] * mF[13] * mF[3 ] - mF[12] * mF[1 ] * mF[11] +
				 mF[0 ] * mF[13] * mF[11] + mF[8 ] * mF[1 ] * mF[15] - mF[0 ] * mF[9 ] * mF[15]) * fDet;
	out[10] =  (-mF[12] * mF[5 ] * mF[3 ] + mF[4 ] * mF[13] * mF[3 ] + mF[12] * mF[1 ] * mF[7 ] -
				 mF[0 ] * mF[13] * mF[7 ] - mF[4 ] * mF[1 ] * mF[15] + mF[0 ] * mF[5 ] * mF[15]) * fDet;
    out[11] =  ( mF[8 ] * mF[5 ] * mF[3 ] - mF[4 ] * mF[9 ] * mF[3 ] - mF[8 ] * mF[1 ] * mF[7 ] +
				 mF[0 ] * mF[9 ] * mF[7 ] + mF[4 ] * mF[1 ] * mF[11] - mF[0 ] * mF[5 ] * mF[11]) * fDet;

    out[12] =  ( mF[12] * mF[9 ] * mF[6 ] - mF[8 ] * mF[13] * mF[6 ] - mF[12] * mF[5 ] * mF[10] +
				 mF[4 ] * mF[13] * mF[10] + mF[8 ] * mF[5 ] * mF[14] - mF[4 ] * mF[9 ] * mF[14]) * fDet;
    out[13] =  (-mF[12] * mF[9 ] * mF[2 ] + mF[8 ] * mF[13] * mF[2 ] + mF[12] * mF[1 ] * mF[10] -
				 mF[0 ] * mF[13] * mF[10] - mF[8 ] * mF[1 ] * mF[14] + mF[0 ] * mF[9 ] * mF[14]) * fDet;
    out[14] =  ( mF[12] * mF[5 ] * mF[2 ] - mF[4 ] * mF[13] * mF[2 ] - mF[12] * mF[1 ] * mF[6 ] +
				 mF[0 ] * mF[13] * mF[6 ] + mF[4 ] * mF[1 ] * mF[14] - mF[0 ] * mF[5 ] * mF[14]) * fDet;
    out[15] =  (-mF[8 ] * mF[5 ] * mF[2 ] + mF[4 ] * mF[9 ] * mF[2 ] + mF[8 ] * mF[1 ] * mF[6 ] -
				 mF[0 ] * mF[9 ] * mF[6 ] - mF[4 ] * mF[1 ] * mF[10] + mF[0 ] * mF[5 ] * mF[10]) * fDet;

	memcpy(mF, out, 64);
}

//============================================================================================================
// Converts a 3D position into 0-1 range screen space
//============================================================================================================

Vector2f Matrix44::Project (const Vector3f& pos) const
{
	Vector2f out ( pos.x * mF[0] + pos.y * mF[4] + pos.z * mF[ 8] + mF[12],
				   pos.x * mF[1] + pos.y * mF[5] + pos.z * mF[ 9] + mF[13] );
	float factor ( pos.x * mF[3] + pos.y * mF[7] + pos.z * mF[11] + mF[15] );

	if (Float::IsNotZero(factor))
	{
		out.x = out.x / factor * 0.5f + 0.5f;
		out.y = out.y / factor * 0.5f + 0.5f;
	}
	else
	{
		out.x = 0.5f;
		out.y = 0.5f;
	}

	return out;
}

//============================================================================================================
// Converts screen space 0-1 range coordinates into 3D space
//============================================================================================================

Vector3f Matrix44::Unproject (const Vector2f& pos, float depth) const
{
	float px = pos.x * 2.0f - 1.0f;
	float py = pos.y * 2.0f - 1.0f;
	float pz = depth * 2.0f - 1.0f;

	Vector3f out ( px * mF[0] + py * mF[4] + pz * mF[ 8] + mF[12],
				   px * mF[1] + py * mF[5] + pz * mF[ 9] + mF[13],
				   px * mF[2] + py * mF[6] + pz * mF[10] + mF[14] );
	float factor ( px * mF[3] + py * mF[7] + pz * mF[11] + mF[15] );

	if (Float::IsNotZero(factor)) out /= factor;
	return out;
}

//============================================================================================================
// Retrieves the 8 corners of the projection matrix
//============================================================================================================

void Matrix44::GetCorners (Vector3f corners[8]) const
{
	float factor[8];

	// 0 0 0
	{
		Vector3f& v = corners[0];
		v.x			= mF[12] - mF[0] - mF[4] - mF[ 8];
		v.y			= mF[13] - mF[1] - mF[5] - mF[ 9];
		v.z			= mF[14] - mF[2] - mF[6] - mF[10];
		factor[0]	= mF[15] - mF[3] - mF[7] - mF[11];
	}

	// 1 0 0
	{
		Vector3f& v = corners[1];
		v.x			= mF[12] + mF[0] - mF[4] - mF[ 8];
		v.y			= mF[13] + mF[1] - mF[5] - mF[ 9];
		v.z			= mF[14] + mF[2] - mF[6] - mF[10];
		factor[1]	= mF[15] + mF[3] - mF[7] - mF[11];
	}

	// 0 1 0
	{
		Vector3f& v = corners[2];
		v.x			= mF[12] - mF[0] + mF[4] - mF[ 8];
		v.y			= mF[13] - mF[1] + mF[5] - mF[ 9];
		v.z			= mF[14] - mF[2] + mF[6] - mF[10];
		factor[2]	= mF[15] - mF[3] + mF[7] - mF[11];
	}

	// 1 1 0
	{
		Vector3f& v = corners[3];
		v.x			= mF[12] + mF[0] + mF[4] - mF[ 8];
		v.y			= mF[13] + mF[1] + mF[5] - mF[ 9];
		v.z			= mF[14] + mF[2] + mF[6] - mF[10];
		factor[3]	= mF[15] + mF[3] + mF[7] - mF[11];
	}

	// 0 0 1
	{
		Vector3f& v = corners[4];
		v.x			= mF[12] - mF[0] - mF[4] + mF[ 8];
		v.y			= mF[13] - mF[1] - mF[5] + mF[ 9];
		v.z			= mF[14] - mF[2] - mF[6] + mF[10];
		factor[4]	= mF[15] - mF[3] - mF[7] + mF[11];
	}

	// 1 0 1
	{
		Vector3f& v = corners[5];
		v.x			= mF[12] + mF[0] - mF[4] + mF[ 8];
		v.y			= mF[13] + mF[1] - mF[5] + mF[ 9];
		v.z			= mF[14] + mF[2] - mF[6] + mF[10];
		factor[5]	= mF[15] + mF[3] - mF[7] + mF[11];
	}

	// 0 1 1
	{
		Vector3f& v = corners[6];
		v.x			= mF[12] - mF[0] + mF[4] + mF[ 8];
		v.y			= mF[13] - mF[1] + mF[5] + mF[ 9];
		v.z			= mF[14] - mF[2] + mF[6] + mF[10];
		factor[6]	= mF[15] - mF[3] + mF[7] + mF[11];
	}

	// 1 1 1
	{
		Vector3f& v = corners[7];
		v.x			= mF[12] + mF[0] + mF[4] + mF[ 8];
		v.y			= mF[13] + mF[1] + mF[5] + mF[ 9];
		v.z			= mF[14] + mF[2] + mF[6] + mF[10];
		factor[7]	= mF[15] + mF[3] + mF[7] + mF[11];
	}

	for (uint i = 0; i < 8; ++i)
	{
		if (Float::IsNotZero(factor[i]))
		{
			corners[i] /= factor[i];
		}
	}
}