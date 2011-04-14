#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Column-major 4x4 matrix
// Author: Michael Lyashenko
//============================================================================================================

struct Matrix44 : public Matrix43
{
public:

	Matrix44 ()													{ SetToIdentity(); }
	Matrix44 (const float mat[16])								{ memcpy(mF, mat, 64); }
	Matrix44 (const Matrix43& mat)								{ *this = mat; }
	Matrix44 (const Matrix44& mat)								{ *this = mat; }
	Matrix44 (float fov,
			  float aspect,
			  float near,
			  float far)										{ SetToProjection(fov, aspect, near, far); }

public:

	// Matrix multiplication differs from its 4x3 counterpart due to 4x3's optimizations
	void	 operator  = (const Matrix43& mat)					{ memcpy(mF, mat.mF, 64); }
	void	 operator  = (const Matrix44& mat)					{ memcpy(mF, mat.mF, 64); }
	Matrix44 operator  * (const Matrix43& mat) const;
	Matrix44 operator  * (const Matrix44& mat) const			{ Matrix44 out(*this); out *= mat; return out; }
	void	 operator *= (const Matrix43& mat);
	void	 operator *= (const Matrix44& mat);

	// 4x4 matrix is capable of being the projection matrix
	void SetToProjection (	float fov,							// Field of view in degrees
							float aspect,						// Aspect ratio (screen width divided by height)
							float near,							// Near clipping plane
							float far );						// Far clipping plane

	// Inversion on a 4x4 matrix is different than 4x3 due to optimizations of 4x3
	void Invert();

	// Converts a 3D position into 0-1 range screen space
	// NOTE: To be used with a Model-View-Projection Matrix
	Vector2f Project (const Vector3f& pos) const;

	// Converts screen space 0-1 range coordinates and non-linear depth into 3D space
	// NOTE: To be used with an Inverse World-View-Projection Matrix
	Vector3f Unproject (const Vector3f& pos) const;

	// Retrieves the 8 corners of the projection matrix
	// NOTE: To be used with an Inverse Model-View-Projection Matrix
	void GetCorners (Vector3f corners[8]) const;
};

//============================================================================================================
// Transforms the vertex: ~27 arithmetic operations
//============================================================================================================

inline void	Vector3f::operator *=(const Matrix44& mat)
{
	Set(	 x * mat[0] + y * mat[4] + z * mat[8]  + mat[12],
			 x * mat[1] + y * mat[5] + z * mat[9]  + mat[13],
			 x * mat[2] + y * mat[6] + z * mat[10] + mat[14]);
	float f (x * mat[3] + y * mat[7] + z * mat[11] + mat[15]);
	if (Float::IsNotZero(f)) *this /= f;
}

//============================================================================================================

inline Vector3f Vector3f::operator *(const Matrix44& mat) const
{
	Vector3f out (x * mat[0] + y * mat[4] + z * mat[8]  + mat[12],
				  x * mat[1] + y * mat[5] + z * mat[9]  + mat[13],
				  x * mat[2] + y * mat[6] + z * mat[10] + mat[14]);
	float factor (x * mat[3] + y * mat[7] + z * mat[11] + mat[15]);
	if (Float::IsNotZero(factor)) out /= factor;
	return out;
}