#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

//============================================================================================================
// Seems like the standard math library is missing this function
//============================================================================================================

inline float log2 (float x)
{
  static const float multiplier = 1.0f / LOG(2.0f);
  return LOG(x) * multiplier;
}

//============================================================================================================
// Checks if the given integer is a power of two
//============================================================================================================

inline bool IsPowerOfTwo(uint iVal)
{
	uint i = 2;
	while (i < iVal) i = i << 1;
	return (i == iVal);
}

//============================================================================================================
// Takes an XY position and width of the array and returns the wrapped index inside the array
//============================================================================================================

inline uint Wrap(int val, int maximum)
{
	while (val >= maximum)	val -= maximum;
	while (val < 0)			val += maximum;
	return (uint)val;
}

//============================================================================================================
// Takes an XY position and width of the array and returns the clamped index inside the array
//============================================================================================================

inline uint Clamp(int val, int maximum)
{
	return (val < 0) ? 0 : (val < maximum ? val : maximum - 1);
}

//============================================================================================================
// Helper function for wrapping of angles
//============================================================================================================

inline void WrapAngle(float &fAngle)
{
	while (fAngle >  PI) fAngle -= TWOPI;
	while (fAngle < -PI) fAngle += TWOPI;
}

//============================================================================================================
// Helper functions for floating point value operations
//============================================================================================================

namespace Float
{
	struct OrInt
	{
		union
		{
			int32	i;
			float	f;
		};

		OrInt(double d)  : f((float)d) {}
		OrInt(float val) : f(val) {}
		OrInt(int32 val) : i(val) {}
	};

	const double	_floorfix	= 68719476736.0 * 1.5;
	const OrInt		_bias		= (int32)(((23 + 127) << 23) + (1 << 22));

	inline float	Abs			(OrInt val)							{ val.i &= 0x7FFFFFFF; return val.f; }
	inline float	Clamp		(float val, float low, float high)	{ return (val < low) ? low : ((val > high) ? high : val); }
	inline bool		IsZero		(float val)							{ return Abs(val) < FLOAT_TOLERANCE; }
	inline bool		IsNotZero	(float val)							{ return Abs(val) > FLOAT_TOLERANCE; }
	inline bool		IsEqual		(float f0, float f1)				{ return Abs(f1 - f0) < FLOAT_TOLERANCE; }
	inline bool		IsNotEqual	(float f0, float f1)				{ return Abs(f1 - f0) > FLOAT_TOLERANCE; }
	inline bool		IsNegative	(OrInt val)							{ return (val.i & 0x80000000) != 0; }
	inline bool		IsPositive	(OrInt val)							{ return (val.i & 0x80000000) == 0; }
	inline float	Min			(float f0, float f1)				{ return (f0 < f1 ? f0 : f1); }
	inline float	Max			(float f0, float f1)				{ return (f0 > f1 ? f0 : f1); }
	inline float	Asin		(float val)							{ return (val < -1.0f ? -HALFPI : (val > 1.0f ? HALFPI : ASIN(val))); }
	inline float	Acos		(float val)							{ return (val < -1.0f ? PI : (val > 1.0f ? 0.0f : ACOS(val))); }
	inline float	Atan2		(float y, float x)					{ return ATAN2(y, x); }

	inline int FloorToInt (double val)
	{
		val += _floorfix;
		return (*(int32*)&val) >> 16;
	}

	inline uint	FloorToUInt	(double val)
	{
		val += _floorfix;
		return (uint)(0x7FFFFFFF & ((*(int32*)&val) >> 16));
	}

	inline float Floor (double val)
	{
		val += _floorfix;
		return (float)((*(int32*)&val) >> 16);
	}

	inline float Ceil (double val)
	{
		val += 1.0f;
		val += _floorfix;
		return (float)((*(int32*)&val) >> 16);
	}

	inline int RoundToInt (OrInt val)
	{
		val.f += _bias.f;
		val.i -= _bias.i;
		return val.i;
	}

	inline uint	RoundToUInt	(OrInt val)
	{
		val.f += _bias.f;
		val.i -= _bias.i;
		return (uint)val.i;
	}

	inline float Round (OrInt val)
	{
		val.f += _bias.f;
		val.i -= _bias.i;
		return (float)val.i;
	}

	inline float Fract (float val)
	{
		return val - Floor(val);
	}

	inline float Sqrt (float val)
	{
		return SQRT(val);
	}

#ifdef R5_USE_INVSQRT
	// John Carmack's implementation
	inline float InvSqrt (float val)
	{
		float half = val * 0.5f;
		int32& i = (int32&)val;
		i = 0x5f3759d5 - (i >> 1);
		return val * (1.5f - half * val * val);
	}
#else
	inline float InvSqrt (float val) { return 1.0f / SQRT(val); }
#endif

	// Table version is way too imprecise for my liking
	inline float Sin (float rad) { return SIN(rad); }
	inline float Cos (float rad) { return COS(rad); }

	inline byte RoundToByte (float val)
	{
		uint ret = RoundToUInt(val);
		return (byte)((ret < 0xFF) ? ret : 0xFF);
	}

	inline byte ToRangeByte (float val)
	{
		int iv = Float::RoundToInt(val * 255.0f);
		if		(iv <   0) return 0;
		else if (iv > 255) return 255;
		return (byte)iv;
	}

	inline byte ToNormalMapByte (float val)
	{
		int iv = Float::RoundToInt(0.5f + (val + 1.0f) * 127.5f);
		if		(iv <   0) return 0;
		else if (iv > 255) return 255;
		return (byte)iv;
	}

	inline float FromRangeByte		(byte in)	{ return Float::Clamp(0.003921568627451f * in, 0.0f, 1.0f); }
	inline float FromNormalMapByte	(byte in)	{ return Float::Clamp(0.0078431372549f * in - 1.0f, 0.0f, 1.0f); }

	// Rounds the float down to a specific precision, so 0.39 with precision of 0.25 becomes 0.5
	float Round (float val, float precision);
};