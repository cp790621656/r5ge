#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// 128-bit color
//============================================================================================================

struct Color4ub;
struct Color4f
{
	float r, g, b, a;
	
	Color4f()											: r(0.0f),	g(0.0f),	b(0.0f),	a(0.0f)		{}
	Color4f(float val)									: r(val),	g(val),		b(val),		a(1.0f)		{}
	Color4f(const Color4ub& c);							// Inlined in Color4ub.h
	Color4f(float R, float G, float B, float A = 1.0f)	: r(R),		g(G),		b(B),		a(A)		{}
	Color4f(const Color3f& c, float alpha = 1.0f)		{ r = c.r;  g = c.g;	b = c.b;	a = alpha;	}
	Color4f(const Color4f& c)							{ r = c.r;	g = c.g;	b = c.b;	a = c.a;	}
	Color4f(const float* f)								{ r = f[0];	g = f[1];	b = f[2];	a = f[3];	}
	
	operator const float*() const						{ return &r; }

	void operator  = (const Color3f& c)					{ r = c.r;  g = c.g;	b = c.b;	a = 1.0f;	}
	void operator  = (const Color4f& v)					{ r = v.r;	g = v.g;	b = v.b;	a = v.a;	}
	void operator  = (const Color4ub& v);				// Inlined in Color4ub.h
	void operator  = (float f)							{ r = f; g = f; b = f; a = f; }
	void operator += (float f)							{ r += f; g += f; b += f; }
	void operator *= (float f)							{ r *= f; g *= f; b *= f; }
	bool operator == (const Color4f& v) const			{ return (r == v.r && g == v.g && b == v.b && a == v.a); }
	bool operator != (const Color4f& v) const			{ return (r != v.r || g != v.g || b != v.b || a != v.a); }
	
	Color4f operator *(float f) const					{ return Color4f(r * f, g * f, b * f, a); }
	Color4f operator *(const Color4f& v) const			{ return Color4f(r * v.r, g * v.g, b * v.b, a * v.a); }
	Color4f operator +(float f) const					{ return Color4f(r + f, g + f, b + f, a); }
	Color4f operator +(const Color4f& v) const			{ return Color4f(r + v.r, g + v.g, b + v.b, a + v.a); }

	bool IsVisible()	const							{ return (a > 0.0f); }
	bool IsVisibleRGB() const							{ return (r != 0 || g != 0 || b != 0); }

	void Set(float R, float G, float B, float A = 1.0f)	{ r = R; g = G; b = B; a = A; }

	void SetFromBytes(byte R, byte G, byte B, byte A = 255)
	{
		r = Float::FromRangeByte(R);
		g = Float::FromRangeByte(G);
		b = Float::FromRangeByte(B);
		a = Float::FromRangeByte(A);
	}

	void LimitToLDR()
	{
		r = Float::Clamp(r, 0.0f, 1.0f);
		g = Float::Clamp(g, 0.0f, 1.0f);
		b = Float::Clamp(b, 0.0f, 1.0f);
		a = Float::Clamp(a, 0.0f, 1.0f);
	}

	void Normalize()
	{
		float val = (r > g ? (r > b ? r : b) : (g > b ? g : b));

		if (Float::IsZero(val))
		{
			r = 1.0f;
			g = 1.0f;
			b = 1.0f;
			a = Float::Clamp(a, 0.0f, 1.0f);
		}
		else
		{
			val = 1.0f / val;
			r *= val;
			g *= val;
			b *= val;
			a  = Float::Clamp(a, 0.0f, 1.0f);
		}
	}
};