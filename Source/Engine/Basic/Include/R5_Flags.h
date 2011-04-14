#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// A very simple embeddable flag class
// Author: Michael Lyashenko
//============================================================================================================

class Flags
{
protected:

	uint mVal;

public:
	Flags() : mVal(0) {}

	void Set (uint flags)				{ mVal = flags; }
	void Set (uint flags, bool value)	{ if (value) mVal |= flags; else mVal &= ~flags; }
	uint Get () const					{ return mVal; }
	bool Get (uint flags) const			{ return (mVal & flags) != 0; }
	void Include (const Flags& flags)	{ mVal |= flags.mVal; }
	void Clear()						{ mVal = 0; }
};