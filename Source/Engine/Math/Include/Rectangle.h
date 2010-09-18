#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic templated rectangle
//============================================================================================================

template <typename Type>
struct Rectangle
{
	Type left;
	Type right;
	Type top;
	Type bottom;

	Rectangle() : left(0), right(0), top(0), bottom(0) {}

	bool operator == (const Rectangle<Type>& rect) const
	{
		return  left	== rect.left	&&
				right	== rect.right	&&
				top		== rect.top		&&
				bottom	== rect.bottom;
	}

	bool operator != (const Rectangle<Type>& rect) const
	{
		return  left	!= rect.left	||
				right	!= rect.right	||
				top		!= rect.top		||
				bottom	!= rect.bottom;
	}

	inline Type GetWidth()  const { return right - left; }
	inline Type GetHeight() const { return bottom - top; }

	inline void Set (Type x0, Type y0, Type x1, Type y1)
	{
		left	= x0;
		top		= y0;
		right	= x1;
		bottom	= y1;
	}
};