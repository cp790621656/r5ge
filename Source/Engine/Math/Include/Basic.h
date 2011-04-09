#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

//============================================================================================================
// Seems like the standard math library is missing this function
//============================================================================================================

inline float log2 (const float& x)
{
  static const float multiplier = 1.0f / LOG(2.0f);
  return LOG(x) * multiplier;
}

//============================================================================================================
// Checks if the given integer is a power of two
//============================================================================================================

inline bool IsPowerOfTwo (const uint& val)
{
	uint i = 2;
	while (i < val) i = i << 1;
	return (i == val);
}

//============================================================================================================
// Rounds the number up to the next power-of-two
//============================================================================================================

inline uint NextPowerOfTwo (const uint& val)
{
	uint i = 2;
	while (i < val) i = i << 1;
	return i;
}

//============================================================================================================
// Takes an index as well as the upper bound of the array and returns a wrapped index
//============================================================================================================

inline uint WrapIndex (int val, const int& maximum)
{
	while (val >= maximum)	val -= maximum;
	while (val < 0)			val += maximum;
	return (uint)val;
}

//============================================================================================================
// Takes an index as well as the upper bound of the array and returns a clamped index
//============================================================================================================

inline uint ClampIndex (const int& val, const int& maximum)
{
	return (val < 0) ? 0 : (val < maximum ? val : maximum - 1);
}

//============================================================================================================
// Generic templated clamp function
//============================================================================================================

template <typename Type> inline Type Clamp (const Type& val, const Type& min, const Type& max)
{
	return (val < min) ? min : (val < max ? val : max);
}

//============================================================================================================
// Templated min/max
//============================================================================================================

template<typename Type> inline Type Min (const Type& f0, const Type& f1) { return (f0 < f1 ? f0 : f1); }
template<typename Type> inline Type Max (const Type& f0, const Type& f1) { return (f0 > f1 ? f0 : f1); }

//============================================================================================================
// Helper function for wrapping of angles
//============================================================================================================

inline float WrapAngle (float radAngle)
{
	while (radAngle >  PI) radAngle -= TWOPI;
	while (radAngle < -PI) radAngle += TWOPI;
	return radAngle;
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

		OrInt (const double& d)  : f((float)d) {}
		OrInt (const float& val) : f(val) {}
		OrInt (const int32& val) : i(val) {}
	};

	const double	_floorfix	= 68719476736.0 * 1.5;
	const OrInt		_bias		= (int32)(((23 + 127) << 23) + (1 << 22));

	inline float	Abs			(OrInt val)							{ val.i &= 0x7FFFFFFF; return val.f; }
	inline float	Clamp		(const float& val, const float& low, const float& high)	{ return (val < low) ? low : ((val > high) ? high : val); }
	inline bool		IsZero		(const float& val)					{ return Abs(val) < FLOAT_TOLERANCE; }
	inline bool		IsNotZero	(const float& val)					{ return Abs(val) > FLOAT_TOLERANCE; }
	inline bool		IsEqual		(const float& f0, const float& f1)	{ return Abs(f1 - f0) < FLOAT_TOLERANCE; }
	inline bool		IsNotEqual	(const float& f0, const float& f1)	{ return Abs(f1 - f0) > FLOAT_TOLERANCE; }
	inline bool		IsNegative	(const OrInt& val)					{ return (val.i & 0x80000000) != 0; }
	inline bool		IsPositive	(const OrInt& val)					{ return (val.i & 0x80000000) == 0; }
	inline float	Asin		(const float& val)					{ return (val > -1.0f ? (val < 1.0f ? ASIN(val) : HALFPI) : -HALFPI); }
	inline float	Acos		(const float& val)					{ return (val > -1.0f ? (val < 1.0f ? ACOS(val) : 0.0f) : PI); }
	inline float	Atan2		(const float& y, const float& x)	{ return ATAN2(y, x); }

	inline bool	IsProductPositive (const OrInt& v0, const OrInt& v1)
	{
		return (v0.i & 0x80000000) == (v1.i & 0x80000000);
	}

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

	inline float Fract (const float& val)
	{
		return val - Floor(val);
	}

	inline float Sqrt (const float& val)
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
	inline float InvSqrt (const float& val) { return 1.0f / SQRT(val); }
#endif

	// Table version is way too imprecise for my liking
	inline float Sin (const float& rad) { return SIN(rad); }
	inline float Cos (const float& rad) { return COS(rad); }

	inline byte RoundToByte (const float& val)
	{
		uint ret = RoundToUInt(val);
		return (byte)((ret < 0xFF) ? ret : 0xFF);
	}

	inline byte ToRangeByte (const float& val)
	{
		return Float::RoundToByte(val * 255.0f);
	}

	inline byte ToNormalMapByte (const float& val)
	{
		return Float::RoundToByte(val * 127.5f + 127.5f);
	}

	inline float FromRangeByte		(const byte& in)	{ return Float::Clamp(0.003921568627451f * in, 0.0f, 1.0f); }
	inline float FromNormalMapByte	(const byte& in)	{ return Float::Clamp(0.0078431372549f * in - 1.0f, -1.0f, 1.0f); }

	// Rounds the float down to a specific precision, so 0.39 with precision of 0.25 becomes 0.5
	float Round (float val, const float& precision);
};