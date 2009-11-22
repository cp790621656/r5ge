#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// A very simple embeddable flag class
//============================================================================================================

class Flags
{
protected:

	uint mVal;

public:
	Flags() : mVal(0) {}

	inline void Set (uint flags)				{ mVal = flags; }
	inline void Set (uint flags, bool value)	{ if (value) mVal |= flags; else mVal &= ~flags; }
	inline uint Get () const					{ return mVal; }
	inline bool Get (uint flags) const			{ return (mVal & flags) != 0; }
};