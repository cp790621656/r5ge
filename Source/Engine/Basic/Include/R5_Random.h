#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Cross platform pseudo-random number generator
// Based on: http://www.codeproject.com/KB/cs/fastrandom.aspx
// Author: Michael Lyashenko
//============================================================================================================

class Random
{
private:

	static const uint B = 1842502087;
	static const uint C = 1357980759;
	static const uint D = 273326509;

	uint a, b, c, d;

public:

	Random();
	Random(uint val) { SetSeed(val); }

	// Random uinteger ranging from 0 to 0xFFFFFFFF
	inline uint GenerateUint()
	{
		uint t = (a ^ (a << 11));
		a = b;
		b = c;
		c = d;
		return d = (d ^ (d >> 19)) ^ (t ^ (t >> 8));
	}

	// Random single precision floating point value ranging from 0.0 to 1.0
	inline float GenerateFloat()
	{
		return 0.00000000023283064370807974f * GenerateUint();
	}

	// Random single precision floating point value ranging from -1.0 to 1.0
	inline float GenerateRangeFloat()
	{
		uint val = GenerateUint();
		return 0.00000000046566128741615948f * (*(int*)&val);
	}

	// Random double precision floating point value ranging from 0.0 to 1.0
	inline double GenerateDouble()
	{
		return 0.00000000023283064370807974 * GenerateUint();
	}

	// Random double precision floating point value ranging from -1.0 to 1.0
	inline double GenerateRangeDouble()
	{
		uint val = GenerateUint();
		return 0.00000000046566128741615948 * (*(int*)&val);
	}

	// Resets the random seed to the specified value
	void SetSeed (uint val);
};