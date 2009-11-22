#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Unified color, stored in both 32-bit as well as 128-bit formats.
// This is an optimization class mainly -- seeing as equality/nonequality operations are much faster with
// 32-bit colors, but the videocard often expects them to be in floating-point format.
//============================================================================================================

class Color
{
	Color4ub	mUb;
	Color4f		mF;

public:

	Color()					 : mUb(0)	{ mF.Set(0.0f, 0.0f, 0.0f, 0.0f); }
	Color(const Color4ub& c) : mUb(c)	{ mF  = c; }
	Color(const Color4f&  c) : mF(c)	{ mUb = c; }
	Color(const Color& c)	 : mUb(c.mUb), mF(c.mF) {}

	void			operator = (const Color4ub& c)		{ mUb = c; mF = c; }
	void			operator = (const Color4f&  c)		{ mUb = c; mF = c; }
	const float*	operator = (const Color& c)			{ mUb = c.mUb; mF = c.mF; return mF; }
	bool			operator ==(const Color& c) const	{ return mUb == c.mUb; }
	bool			operator !=(const Color& c) const	{ return mUb != c.mUb; }

	operator const Color4ub& ()	const	{ return mUb; }
	operator const Color4f&  ()	const	{ return mF;  }
	operator const float*()		const	{ return &mF[0]; }

	void Clear() { mUb = 0; mF.Set(0.0f, 0.0f, 0.0f, 0.0f); }

	const Color4ub& GetColor4ub() const { return mUb; }
	const Color4f&  GetColor4f()  const { return mF;  }
};