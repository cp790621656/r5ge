#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
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

	bool operator == (const Flags& f) const		{ return mVal == f.mVal; }
	bool operator != (const Flags& f) const		{ return mVal != f.mVal; }
	bool operator == (const uint& val) const	{ return mVal == val; }
	bool operator != (const uint& val) const	{ return mVal != val; }
};