#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Column-major 4x3 matrix. Note that it is actually a 4x4 matrix, but only 4x3 part of it is used.
// This matrix is suitable for everything that does not involve projection, such as view and world transforms.
//============================================================================================================
// Column-major matrix:            Row-major matrix:
//  (Math & OpenGL)				  (Graphics & DirectX)
//============================================================================================================
//  0   4   8  12					 0   1   2   3
//  1   5   9  13					 4   5   6   7
//  2   6  10  14					 8   9  10  11
//  3   7  11  15					12  13  14  15
//============================================================================================================

struct Matrix44;
struct Matrix43
{
public:

	union
	{
		float mF[16];

		struct
		{
			float mColumn1[4];
			float mColumn2[4];
			float mColumn3[4];

			union
			{
				float mColumn4[4];
				struct { float x, y, z, w; };
			};
		};
	};

public:
	
	Matrix43 ()													{ SetToIdentity(); }
	Matrix43 (const float mat[16])								{ memcpy(mF, mat, 64); }
	Matrix43 (const Matrix43& mat)								{ *this = mat; }
	Matrix43 (float scale)										{ SetToIdentity(); mF[0] = scale; mF[5] = scale; mF[10] = scale; }
	Matrix43 (const Vector3f& v)								{ SetToIdentity(); x = v.x; y = v.y; z = v.z; }
	Matrix43 (const Quaternion& q);
	Matrix43 (float width, float height)						{ SetToScreen(width, height); }

	Matrix43 (const Vector3f& eye,
			  const Vector3f& dir,
			  const Vector3f& up )								{ SetToView(eye, dir, up); }

	Matrix43 (const Vector3f& pos, float scale)					{ SetToTransform(pos, scale); }

	Matrix43 (const Vector3f&	pos,
			  const Quaternion&	rot,
			  float				scale)							{ SetToTransform(pos, rot, scale); }

private: // Invalid functions:

	Matrix43		 (const Matrix44& mat) {}					// Matrix44 cannot be converted back to Matrix43 due to projection
	void operator  = (const Matrix44& mat) {}					// Same reason as above
	void operator *= (const Matrix44& mat) {}					// Result of this operation is a Matrix44, and cannot be assigned to Matrix43

public:

	operator const void*() const								{ return mF; }
	float& operator [] (uint index)								{ return mF[index]; }
	const float& operator [] (uint index) const					{ return mF[index]; }

	void operator  = (const Matrix43& mat)						{ memcpy(mF, mat.mF, 64); }
	void operator  = (const Quaternion& q);
	void operator *= (const Matrix43& mat);
	void operator *= (const Quaternion& q)						{ Matrix43 mMat(q);  *this *= mMat; }
	
	Matrix43 operator *(const Matrix43& mat) const;
	Matrix44 operator *(const Matrix44& mat) const;

	void SetToView ( const Vector3f& eye,						// World-space eye position
					 const Vector3f& dir,						// Normalized lookAt direction
					 const Vector3f& up );						// Normalized up vector

	void SetToScreen (float width, float height);

	// Sets the matrix transform using a translation and a scale component
	void SetToTransform (const Vector3f& pos, float scale);

	// Most optimized function to use that combines translation, rotation, and scaling
	void SetToTransform (const Vector3f& pos,
						 const Quaternion& rot,
						 float scale);

	// Optimized matrix setting function meant for bone transforms
	void SetToTransform	(const Vector3f&	invBindPos,
						 const Vector3f&	absolutePos,
						 const Quaternion&	invBindRot,
						 const Quaternion&	absoluteRot );

	bool IsIdentity();
	void SetToIdentity();
	void Translate		(const Vector3f& offset)				{ x += offset.x;	y += offset.y;		z += offset.z; }
	void PreTranslate	(const Vector3f& offset);
	void Rotate			(const Vector3f& axis, float radAngle);
	void Scale			(float scale);
	void Scale			(const Vector3f& scale);
	void PreScale		(float scale);
	void PreScale		(const Vector3f& scale);
	void Invert();
	void InvertOrthogonal()										{ Transpose(); FlipTranslation(); }
	void Transpose();
	void FlipTranslation()										{ x = -x;  y = -y;  z = -z; }
	void ClearTranslation()										{ x = 0.0f; y = 0.0f; z = 0.0f; }
	void ReplaceScaling (float scale = 1.0f);
};

//============================================================================================================
// Transforms the vertex: 8 arithmetic operations for Vector2f, 18 for Vector3f
//============================================================================================================

inline void Vector2f::operator *=(const Matrix43& mat)
{
	Set(x * mat[0] + y * mat[4] + mat[12],
		x * mat[1] + y * mat[5] + mat[13]);
}

//============================================================================================================

inline void Vector3f::operator *=(const Matrix43& mat)
{
	Set(x * mat[0] + y * mat[4] + z * mat[8]  + mat[12],
		x * mat[1] + y * mat[5] + z * mat[9]  + mat[13],
		x * mat[2] + y * mat[6] + z * mat[10] + mat[14]);
}

//============================================================================================================

inline Vector3f Vector3f::operator *(const Matrix43& mat) const
{
	return Vector3f(x * mat[0] + y * mat[4] + z * mat[8]  + mat[12],
					x * mat[1] + y * mat[5] + z * mat[9]  + mat[13],
					x * mat[2] + y * mat[6] + z * mat[10] + mat[14]);
}

//============================================================================================================
// Only rotates: 6 arithmetic operations for Vector2f, 15 for Vector3f
//============================================================================================================

inline void Vector2f::operator %=(const Matrix43& mat)
{
	Set(x * mat[0]  +  y * mat[4],
		x * mat[1]  +  y * mat[5]);
}

//============================================================================================================

inline void Vector3f::operator %=(const Matrix43& mat)
{
	Set(x * mat[0]  +  y * mat[4]  +  z * mat[8],
		x * mat[1]  +  y * mat[5]  +  z * mat[9],
		x * mat[2]  +  y * mat[6]  +  z * mat[10]);
}

//============================================================================================================

inline Vector3f Vector3f::operator %(const Matrix43& mat) const
{
	return  Vector3f(x * mat[0]  +  y * mat[4]  +  z * mat[8],
					 x * mat[1]  +  y * mat[5]  +  z * mat[9],
					 x * mat[2]  +  y * mat[6]  +  z * mat[10]);
}

//============================================================================================================
// Copies over the translation portion of the matrix
//============================================================================================================

inline void Vector3f::operator = (const Matrix43& mat)
{
	x = mat.x;
	y = mat.y;
	z = mat.z;
}