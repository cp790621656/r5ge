#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic Quaternion implementation designed to work with Z being the 'up' axis
//============================================================================================================

struct Matrix43;
struct Quaternion
{
	float x, y, z, w;

	Quaternion()												: x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quaternion(float f)											: x(f), y(f), z(f), w(f) {}
	Quaternion(float X, float Y, float Z, float W)				: x(X), y(Y), z(Z), w(W) {}

	Quaternion(const Vector3f& direction)						{ SetFromDirection(direction);							}
	Quaternion(const Vector3f& axis, float radAngle)			{ SetFromAxisAngle(axis, radAngle);						}
	Quaternion(float radX, float radY, float radZ)				{ SetFromEuler(Vector3f(radX, radY, radZ));				}
	Quaternion(const Quaternion& v)								{ x = v.x;			y = v.y;	z = v.z;	w = v.w;	}
	Quaternion(const Quaternion& q0, const Quaternion& q1)		{ Combine(q0, q1);										}
	Quaternion(const Quaternion* v)								{ x = v->x;			y = v->y;	z = v->z;	w = v->w;	}
	Quaternion(const float *f)									{ x = f[0];			y = f[1];	z = f[2];	w = f[3];	}
	Quaternion(const Matrix43& mat)								{ *this = mat;											}

	float Dot()	const											{ return (x * x + y * y + z * z + w * w);				}
	float Dot (const Quaternion& q) const						{ return Float::Clamp(x * q.x + y * q.y + z * q.z + w * q.w, -1.0f, 1.0f);	}
	float Magnitude() const										{ return Float::Sqrt(x * x + y * y + z * z + w * w);	}

#ifdef R5_USE_INVSQRT
	void Normalize()											{ (*this) *= Float::InvSqrt(x * x + y * y + z * z + w * w); }
#else
	void Normalize()
	{
		float mag = Float::Sqrt(x * x + y * y + z * z + w * w);

		if ( mag > FLOAT_TOLERANCE )
		{
			mag = 1.0f / mag;
			x *= mag;
			y *= mag;
			z *= mag;
			w *= mag;
		}
		else
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
			w = 1.0f;
		}
	}
#endif

	// I recommend using the '-' operator instead, unless your quaternion is not normalized
	void Invert()												{ Normalize();		x = -x;		y = -y;		z = -z;		}
	void Flip()													{ x = -x;			y = -y;		z = -z;		w = -w;		}
	bool IsIdentity() const										{ return (x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 1.0f); }
	void SetToIdentity()										{ x = y = z = 0;	w = 1;								}
	void Set (float xx, float yy, float zz, float ww)			{ x = xx;			y = yy;		z = zz;		w = ww;		}
	void Combine (const Quaternion& a, const Quaternion& b)		{ x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
																  y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
																  z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
																  w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z; }

	// Sets the quaternion from Pitch (X), Roll (Y), and Yaw (Z) rotations specified in radians
	void SetFromEuler (const Vector3f& rad);
	void SetFromAxisAngle (const Vector3f& vNormalizedAxis, float radAngle);
	void SetFromDirection (const Vector3f& direction);
	
	operator float*()											{ return &x; }
	operator const float*() const								{ return &x; }

	// Conjugate -- in case of unit length quaternion it is also its inverse
	Quaternion operator -() const								{ return Quaternion(-x, -y, -z, w); }

	Vector3f&		xyz()										{ return *((Vector3f*)&x);			}
	const Vector3f& xyz() const									{ return *((const Vector3f*)&x);	}

	void operator  =(float f)									{ x = f;		y = f;		z = f;		w = f;	}
	void operator  =(const Vector3f& v)							{ SetFromDirection(v); }
	void operator  =(const Quaternion& v)						{ x = v.x;		y = v.y;	z = v.z;	w = v.w;	}
	void operator  =(const Matrix43& mat);
	void operator /=(float f)									{ f = 1.0f / f; x *= f;		y *= f;		z *= f;		w *= f; }
	void operator *=(float f)									{ x *= f;		y *= f;		z *= f;		w *= f;	}
	void operator *=(const Matrix43& mat)						{ *this *= Quaternion(mat); }
	void operator *=(const Quaternion& q)						{ Set(w * q.x + x * q.w + y * q.z - z * q.y,
																	  w * q.y - x * q.z + y * q.w + z * q.x,
																	  w * q.z + x * q.y - y * q.x + z * q.w,
																	  w * q.w - x * q.x - y * q.y - z * q.z); }
	
	bool operator ==(const Quaternion& v) const					{ return  ( Float::IsZero(x - v.x) &&
																			Float::IsZero(y - v.y) &&
																			Float::IsZero(z - v.z) &&
																			Float::IsZero(w - v.w) ); }

	bool operator !=(const Quaternion& v) const					{ return !( Float::IsZero(x - v.x) &&
																			Float::IsZero(y - v.y) &&
																			Float::IsZero(z - v.z) &&
																			Float::IsZero(w - v.w) ); }
	
	Quaternion operator *(float f) const						{ return Quaternion(x * f, y * f, z * f, w * f); }
	Quaternion operator *(const Matrix43& mat) const			{ return (*this) * Quaternion(mat); }
	Quaternion operator *(const Quaternion& q)  const			{ return Quaternion(w * q.x + x * q.w + y * q.z - z * q.y,
																					w * q.y - x * q.z + y * q.w + z * q.x,
																					w * q.z + x * q.y - y * q.x + z * q.w,
																					w * q.w - x * q.x - y * q.y - z * q.z); }

	Quaternion operator +(const Quaternion& q) const			{ return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w); }
	Quaternion operator -(const Quaternion& q) const			{ return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w); }

	// Rotate the quaternion using the normalized axis/angle combo
	void Rotate (Vector3f& vNormalizedAxis, float radAngle)		{ *this *= Quaternion(vNormalizedAxis, radAngle); }

	// Conversion to euler angles (in radians)
	Vector3f GetEuler() const;

	// Conversion to axis-angle (angle is in radians)
	void GetAxisAngle (Vector3f& axis, float& radAngle) const;

	// Spheric linear interpolation angle
	float GetSlerpAngle (const Quaternion& q) const				{ return Float::Acos(Dot(q)); }

public:

	// Vector3f(0, 1, 0) * (*this)
	Vector3f GetForward() const
	{
		return Vector3f(		2.0f * (x * y - z * w),
						 1.0f - 2.0f * (x * x + z * z),
								2.0f * (y * z + x * w) );
	}

	// Vector3f(1, 0, 0) * (*this)
	Vector3f GetRight() const
	{
		return Vector3f( 1.0f - 2.0f * (y * y + z * z),
								2.0f * (x * y + z * w),
								2.0f * (x * z - y * w) );
	}

	// Vector3f(0, 0, 1) * (*this)
	Vector3f GetUp() const
	{
		return Vector3f(		2.0f * (x * z + y * w),
								2.0f * (y * z - x * w),
						 1.0f - 2.0f * (x * x + y * y) );
	}

	// Two-dimensional forward vector
	Vector2f GetFlatForward() const
	{
		Vector2f v (		2.0f * (x * y - z * w),
					 1.0f - 2.0f * (x * x + z * z) );
		v.Normalize();
		return v;
	}
};

//============================================================================================================
// Convenience functions
//============================================================================================================

inline			Vector3f::Vector3f	 (const Quaternion& q)		{ *this = q.GetForward(); }
inline void		Vector3f::operator  =(const Quaternion& q)		{ *this = q.GetForward(); }

//============================================================================================================
// Convenience type, likely good canditate for future refactoring
//============================================================================================================

typedef Quaternion Vector4f;