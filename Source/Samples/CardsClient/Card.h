#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Card information struct
//============================================================================================================

struct Card
{
	byte value;	// For sorting purposes
	byte color;	// Hearts, Diamonds, Clubs, Spades

	Card() : value(0), color(0) {}

	bool operator < (const Card& c)
	{
		if (value  < c.value) return true;
		if (value == c.value) return color < c.color;
		return false;
	}

	operator ushort() const { return ((((ushort)value) << 8) | color); }
	void operator = (ushort val) { value = (byte)(val >> 8); color = (byte)(val & 0xF); }

	void Set (char val, byte c)
	{
		value = val;
		color = c;
	}

	Color4ub GetColor() const
	{
		if (color == 0) return Color4ub(255, 50, 50, 255);
		if (color == 1) return Color4ub(255, 255, 50, 255);
		if (color == 2) return Color4ub(50, 100, 200, 255);
		return Color4ub(50, 50, 50, 255);
	}
};