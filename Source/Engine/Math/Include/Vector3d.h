#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Double precision floating point vector
// NOTE: Unless you are certain you need double precision, I recommend using Vector3f instead.
//============================================================================================================

struct Vector3d
{
	double x, y, z;

	Vector3d()										: x(0.0), y(0.0), z(0.0) {}
	Vector3d(double _x, double _y, double _z)		: x(_x), y(_y), z(_z) {}
	Vector3d(const Vector3f& v)						: x ((double)v.x), y((double)v.y), z((double)v.z) {}
	void Set (double _x, double _y, double _z)		{ x = _x; y = _y; z = _z; }
	
	void Normalize()
	{
		double mag = sqrt((x * x) + (y * y) + (z * z));
		if (fabs(mag) < 0.000001) Set(0.0, 0.0, 1.0);
		else
		{
			double f = 1.0 / mag;
			x *= f;
			y *= f;
			z *= f;
		}
	}

	Vector3d operator - (double val)		const	{ return Vector3d(x - val, y - val, z - val); }
	Vector3d operator + (double val)		const	{ return Vector3d(x + val, y + val, z + val); }
	Vector3d operator * (double val)		const	{ return Vector3d(x * val, y * val, z * val); }
	Vector3d operator / (double val)		const	{ double inv (1.0f / val); return Vector3d(x * inv, y * inv, z * inv); }
	Vector3d operator + (const Vector3d& v) const	{ return Vector3d(x + v.x, y + v.y, z + v.z); }
	Vector3d operator - (const Vector3d& v) const	{ return Vector3d(x - v.x, y - v.y, z - v.z); }
	Vector3d operator * (const Vector3d& v) const	{ return Vector3d(x * v.x, y * v.y, z * v.z); }

	void operator +=(double val)					{ Set(x + val, y + val, z + val); }
	void operator -=(double val)					{ Set(x - val, y - val, z - val); }
	void operator *=(double val)					{ Set(x * val, y * val, z * val); }
	void operator /=(double val)					{ double inv (1.0f / val); Set(x * inv, y * inv, z * inv); }
	void operator +=(const Vector3d& v)				{ Set(x + v.x, y + v.y, z + v.z); }
	void operator -=(const Vector3d& v)				{ Set(x - v.x, y - v.y, z - v.z); }
	void operator *=(const Vector3d& v)				{ Set(x * v.x, y * v.y, z * v.z); }
	operator Vector3f() const						{ return Vector3f((float)x, (float)y, (float)z); }
};