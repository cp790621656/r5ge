#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
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
		if (color == 0) return Color4ub(255, 175, 150, 255);
		if (color == 1) return Color4ub(200, 175, 150, 255);
		if (color == 2) return Color4ub(200, 200, 200, 255);
		return Color4ub(150, 150, 150, 255);
	}
};