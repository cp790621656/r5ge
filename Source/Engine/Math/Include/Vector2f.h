#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// 2D float-based vector
//============================================================================================================

struct Matrix43;
struct Vector2f
{
	union
	{
		struct { float x, y; };
		struct { float u, v; };
	};

	Vector2f()					: x(0),	 y(0)	{}
	Vector2f(float X, float Y)	: x(X),	 y(Y)   {}
	Vector2f(const Vector2f& v) { x = v.x;  y = v.y;  }
	Vector2f(const Vector2i& v) { x = v.x;  y = v.y;  }
	Vector2f(const float* v)	{ x = v[0]; y = v[1]; }
	void Set(float X, float Y)	{ x = X; y = Y; }

	operator bool() const							{ return !(Float::IsZero(x) && Float::IsZero(y)); }
	bool operator == (const Vector2f& v) const		{ return  (Float::IsZero(v.x - x) && Float::IsZero(v.y - y)); }
	bool operator != (const Vector2f& v) const		{ return !(Float::IsZero(v.x - x) && Float::IsZero(v.y - y)); }
	void operator  = (const Vector2f& v)			{ x = v.x;  y = v.y;  }
	void operator  = (const Vector2i& v)			{ x = v.x;  y = v.y;  }
	void operator  = (const float* v)				{ x = v[0];	y = v[1]; }
	void operator  = (float val)					{ x = val;  y = val;  }

	void operator += (const Vector2f& v)			{ x += v.x; y += v.y; }
	void operator -= (const Vector2f& v)			{ x -= v.x; y -= v.y; }
	void operator *= (const Vector2f& v)			{ x *= v.x; y *= v.y; }
	void operator /= (const Vector2f& v)			{ x /= v.x; y /= v.y; }

	void operator += (const Vector2i& v)			{ x += v.x; y += v.y; }
	void operator -= (const Vector2i& v)			{ x -= v.x; y -= v.y; }
	void operator *= (const Vector2i& v)			{ x *= v.x; y *= v.y; }
	void operator /= (const Vector2i& v)			{ x /= v.x; y /= v.y; }
	
	void operator += (float val)					{ x += val; y += val; }
	void operator -= (float val)					{ x -= val; y -= val; }
	void operator *= (float val)					{ x *= val; y *= val; }
	void operator /= (float val)					{ x /= val; y /= val; }

	Vector2f operator +(float val) const			{ return Vector2f(x + val, y + val); }
	Vector2f operator -(float val) const			{ return Vector2f(x - val, y - val); }
	Vector2f operator *(float val) const			{ return Vector2f(x * val, y * val); }
	Vector2f operator /(float val) const			{ return Vector2f(x / val, y / val); }

	Vector2f operator +(const Vector2f& v) const	{ return Vector2f(x + v.x, y + v.y); }
	Vector2f operator -(const Vector2f& v) const	{ return Vector2f(x - v.x, y - v.y); }
	Vector2f operator *(const Vector2f& v) const	{ return Vector2f(x * v.x, y * v.y); }
	Vector2f operator /(const Vector2f& v) const	{ return Vector2f(x / v.x, y / v.y); }
	
	Vector2f operator +(const Vector2i& v) const	{ return Vector2f(x + v.x, y + v.y); }
	Vector2f operator -(const Vector2i& v) const	{ return Vector2f(x - v.x, y - v.y); }
	Vector2f operator *(const Vector2i& v) const	{ return Vector2f(x * v.x, y * v.y); }
	Vector2f operator /(const Vector2i& v) const	{ return Vector2f(x / v.x, y / v.y); }

	void operator *=(const Matrix43& mat);			// Inlined in Matrix43.h
	void operator %=(const Matrix43& mat);			// Inlined in Matrix43.h

	float Dot()										{ return x * x + y * y; }
	float Magnitude()								{ return Float::Sqrt(x * x + y * y); }
	
#ifdef R5_USE_INVSQRT
	void Normalize()								{ (*this) *= Float::InvSqrt(x * x + y * y); }
#else
	void Normalize()
	{
		float mag = Float::Sqrt(x * x + y * y);

		if ( mag > FLOAT_TOLERANCE  )
		{
			x /= mag;
			y /= mag;
		}
		else
		{
			x = 0.0f;
			y = 1.0f;
		}
	}
#endif
};

// Provided for convenience
inline Vector2f	Vector2i::operator *(float val) const { return Vector2f(val * x, val * y); }