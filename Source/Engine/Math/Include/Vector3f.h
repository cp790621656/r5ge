#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// 3D float-based vector
//============================================================================================================

struct Quaternion;
struct Matrix43;
struct Matrix44;

struct Vector3f
{
	float x, y, z;

	Vector3f()									: x(0.0f), y(0.0f), z(0.0f) {}
	Vector3f(float f)							: x(f),	y(f), z(f) {}
	Vector3f(float X, float Y, float Z)			: x(X),	y(Y), z(Z) {}
	Vector3f(const Vector2f& v)					{ x = v.x; y = v.y; z = 0.0f; }
	Vector3f(const Vector3f& v)					{ x = v.x; y = v.y; z = v.z;  }
	Vector3f(const Color4ub& c)					{ *this = c; }
	Vector3f(const Quaternion& q);				// Inlined in Quaternion.h

	operator float*()							{ return &x; }
	operator const float*()	const				{ return &x; }
	operator bool()								{ return !(Float::IsZero(x)		  && Float::IsZero(y)		&& Float::IsZero(z));		}
	operator Vector2f()							{ Vector2f v(x, y);  v.Normalize();  return v; }

	bool operator !() const						{ return  (Float::IsZero(x)		  && Float::IsZero(y)		&& Float::IsZero(z));		}
	bool operator ==(const Vector3f& v) const	{ return  (Float::IsZero(x - v.x) && Float::IsZero(y - v.y) && Float::IsZero(z - v.z));	}
	bool operator !=(const Vector3f& v) const	{ return !(Float::IsZero(x - v.x) && Float::IsZero(y - v.y) && Float::IsZero(z - v.z));	}
	
	void operator *=(float val)					{ x *= val; y *= val; z *= val; }
	void operator /=(float val)					{ val = 1.0f / val; x *= val; y *= val; z *= val; }
	void operator /=(const Vector3f& v)			{ x /= v.x; y /= v.y; z /= v.z; }
	void operator +=(float val)					{ x += val; y += val; z += val; }
	void operator -=(float val)					{ x -= val; y -= val; z -= val; }
	void operator +=(const Vector2f& v)			{ x += v.x; y += v.y; }
	void operator -=(const Vector2f& v)			{ x -= v.x; y -= v.y; }
	void operator +=(const Vector3f& v)			{ x += v.x; y += v.y; z += v.z; }
	void operator -=(const Vector3f& v)			{ x -= v.x; y -= v.y; z -= v.z; }
	void operator /=(ushort iIn)				{ float val = 1.0f / iIn;	x *= val;  y *= val;  z *= val; }
	void operator /=(uint iIn)					{ float val = 1.0f / iIn;	x *= val;  y *= val;  z *= val; }
	void operator *=(const Vector3f& v)			{							x *= v.x;  y *= v.y;  z *= v.z; }
	void operator *=(const Quaternion& q);
	void operator *=(const Matrix43& mat);		// Inlined in Matrix43.h
	void operator %=(const Matrix43& mat);		// Inlined in Matrix43.h
	void operator *=(const Matrix44& mat);		// Inlined in Matrix44.h

	void operator = (float f)					{ x = f;		y = f;			z = f;	  }
	void operator = (const Vector2f& v)			{ x = v.x;		y = v.y;		z = 0;    }
	void operator = (const Vector3f& v)			{ x = v.x;		y = v.y;		z = v.z;  }
	void operator = (const Vector3f* v)			{ x = v->x;		y = v->y;		z = v->z; }
	void operator = (const float* v)			{ x = v[0];		y = v[1];		z = v[2]; }
	void operator = (const Color4ub& c)			{ x = Float::FromNormalMapByte(c.r);
												  y = Float::FromNormalMapByte(c.g);
												  z = Float::FromNormalMapByte(c.b);
												  Normalize(); }
	void operator = (int iIn)					{ x = (float)iIn;	y = (float)iIn;	z = (float)iIn; }
	void operator = (const Quaternion& q);		// Inlined in Quaternion.h
	void operator = (const Matrix43& mat);		// Inlined in Matrix43.h

	Vector3f  operator + (float val)			const		{ return Vector3f(x + val, y + val, z + val); }
	Vector3f  operator - (float val)			const		{ return Vector3f(x - val, y - val, z - val); }
	Vector3f  operator + (const Vector2f& v)	const		{ return Vector3f(x + v.x, y + v.y, z); }
	Vector3f  operator - (const Vector2f& v)	const		{ return Vector3f(x - v.x, y - v.y, z); }
	Vector3f  operator + (const Vector3f& v)	const		{ return Vector3f(v.x + x,	v.y + y,	v.z + z); }
	Vector3f  operator - (const Vector3f& v)	const		{ return Vector3f(x - v.x,	y - v.y,	z - v.z); }
	Vector3f  operator * (float val)			const		{ return Vector3f(x * val,	y * val,	z * val); }
	Vector3f  operator * (const Vector3f& v)	const		{ return Vector3f(x * v.x,	y * v.y,	z * v.z); }
	Vector3f  operator * (const Quaternion& q)	const;
	Vector3f  operator / (const Vector3f& v)	const		{ return Vector3f(x / v.x, y / v.y, z / v.z); }
	Vector3f  operator / (float val)			const		{ val = 1.0f / val; return Vector3f(x * val, y * val, z * val); }
	Vector3f  operator - ()						const		{ return Vector3f(-x, -y, -z); }
	Vector3f  operator * (const Matrix43& mat)	const;		// Inlined in Matrix43.h
	Vector3f  operator % (const Matrix43& mat)	const;		// Inlined in Matrix43.h
	Vector3f  operator * (const Matrix44& mat)	const;		// Inlined in Matrix44.h

	bool operator < (const Vector3f& v) const				{ return x < v.x && y < v.y && z < v.z; }
	bool operator > (const Vector3f& v) const				{ return x > v.x && y > v.y && z > v.z; }

	// Whether this vector has default (all zero) values
	bool  IsZero() const									{ return Float::IsZero( Float::Abs(x) + Float::Abs(y) + Float::Abs(z) ); }
	bool  IsOne() const										{ return Float::IsZero( Float::Abs(x - 1.0f) + Float::Abs(y - 1.0f) + Float::Abs(z - 1.0f) ); }
	void  Flip()											{ x = -x; y = -y; z = -z;									}
	float Sum() const										{ return Float::Abs(x) + Float::Abs(y) + Float::Abs(z);		}
	float Average() const									{ return (x + y + z) * 0.333333f; }
	float Magnitude() const									{ return Float::Sqrt((x * x) + (y * y) + (z * z));			}
	float Dot() const										{ return x * x + y * y + z * z;								}
	float Dot(const Vector3f& v) const						{ return ((x * v.x) + (y * v.y) + (z * v.z));				}
	float GetDistanceTo(const Vector3f& v) const			{ return Vector3f(v.x - x, v.y - y, v.z - z).Magnitude();	}
	float GetDistanceTo(float X, float Y, float Z) const	{ return Vector3f(X - x, Y - y, Z - z).Magnitude();			}
	
	void Cross(const Vector3f& v)							{ Set ( (y * v.z) - (z * v.y),
																	(z * v.x) - (x * v.z),
																	(x * v.y) - (y * v.x)); }
#ifdef R5_USE_INVSQRT
	void Normalize()										{ (*this) *= Float::InvSqrt(x * x + y * y + z * z); }
#else
	void Normalize()
	{
		float mag = Float::Sqrt(x * x + y * y + z * z);

		if (mag > FLOAT_TOLERANCE)
		{
			mag = 1.0f / mag;
			x *= mag;
			y *= mag;
			z *= mag;
		}
		else
		{
			x = 0.0f;
			y = 0.0f;
			z = 1.0f;
		}
	}
#endif

	void Set(float fX, float fY)							{ x  = fX;		y  = fY;				}
	void Set(float fX, float fY, float fZ)					{ x  = fX;		y  = fY;	z  = fZ;	}
	void Add(float fX, float fY, float fZ)					{ x += fX;		y += fY;	z += fZ;	}

	void Rotate(const Vector3f& axis, float radAngle);
	void Interpolation(const Vector3f& v0, const Vector3f& v1, float fFactor) { *this = v0 * (1.0f - fFactor) + v1 * fFactor; }
};

//============================================================================================================
// Convenience function for normal-to-color conversion
//============================================================================================================

inline void	Color4ub::operator =(const Vector3f& v)
{
	r = Float::ToNormalMapByte(v.x);
	g = Float::ToNormalMapByte(v.y);
	b = Float::ToNormalMapByte(v.z);
	a = 255;
}

inline Color3f::Color3f(const Vector3f& c)				{ r = c.x;	g = c.y;	b = c.z; }
inline Color4f::Color4f(const Vector3f& v, float alpha) { r = v.x;	g = v.y;	b = v.z;	a = alpha;	}