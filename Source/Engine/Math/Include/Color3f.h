#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// 96-bit color
//============================================================================================================

struct Color4ub;
struct Color3f
{
	float r, g, b;
	
	Color3f()											: r(0.0f),	g(0.0f),	b(0.0f) {}
	Color3f(float R, float G, float B)					: r(R),		g(G),		b(B)	{}
	Color3f(const Color3f& c)							{ r = c.r;	g = c.g;	b = c.b; }
	Color3f(const Color4ub& c);							// Inlined in Color4ub.h
	Color3f(float val)									{ r = val;	g = val;	b = val; }
	Color3f(int val)									{ *this = val; }
	Color3f(uint val)									{ *this = val; }
	Color3f(float* f)									{ r = f[0]; g = f[1]; b = f[2]; }

	operator const float*() const						{ return &r; }

	void operator  = (int val)							{ *this = (uint)val; }
	void operator  = (uint val)							{ r = Float::FromRangeByte( (val)		& 0xFF );
														  g = Float::FromRangeByte( (val >> 8)  & 0xFF );
														  b = Float::FromRangeByte( (val >> 16) & 0xFF ); }
	void operator  = (const Color4ub& v);				// Inlined in Color4ub.h
	void operator  = (const Color3f& v)					{ r = v.r; g = v.g; b = v.b; }
	void operator  = (float f)							{ r = f; g = f; b = f; }
	void operator += (float f)							{ r += f; g += f; b += f; }
	void operator *= (float f)							{ r *= f; g *= f; b *= f; }
	bool operator == (const Color3f& v) const			{ return (r == v.r && g == v.g && b == v.b); }
	bool operator != (const Color3f& v) const			{ return (r != v.r || g != v.g || b != v.b); }
	
	Color3f operator *(float f) const					{ return Color3f(r * f, g * f, b * f); }
	Color3f operator *(const Color3f& v) const			{ return Color3f(r * v.r, g * v.g, b * v.b); }
	Color3f operator +(float f) const					{ return Color3f(r + f, g + f, b + f); }
	Color3f operator +(const Color3f& v) const			{ return Color3f(r + v.r, g + v.g, b + v.b); }
	
	void Set(float R, float G, float B)					{ r = R; g = G; b = B; }

	void LimitToLDR()
	{
		r = Float::Clamp(r, 0.0f, 1.0f);
		g = Float::Clamp(g, 0.0f, 1.0f);
		b = Float::Clamp(b, 0.0f, 1.0f);
	}

	void Normalize()
	{
		float val = (r > g ? (r > b ? r : b) : (g > b ? g : b));

		if (Float::IsZero(val))
		{
			r = 1.0f;
			g = 1.0f;
			b = 1.0f;
		}
		else
		{
			val = 1.0f / val;
			r *= val;
			g *= val;
			b *= val;
		}
	}
};