#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Cross platform pseudo-random number generator
// Based on: http://www.codeproject.com/KB/cs/fastrandom.aspx
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
	Random(uint val) : a(val), b(B), c(C), d(D) {}

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
		return 0.00000000046566128741615948f * (*reinterpret_cast<int*>(&val));
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
		return 0.00000000046566128741615948 * (*reinterpret_cast<int*>(&val));
	}

	// Resets the random seed to the specified value
	inline void SetSeed (uint val)
	{
		a = val;
		b = B;
		c = C;
		d = D;
	}
};