#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Uniform is a constant value in the shader that is set by the program
//============================================================================================================

struct Uniform
{
	struct Type
	{
		enum
		{
			Invalid			= 0,
			Float1			= 1,	// All the float values are passed by copy
			Float2			= 2,
			Float3			= 3,
			Float4			= 4,
			Float9			= 9,
			Float16			= 16,

			ArrayFloat1		= 33,	// Array values are passed by pointer, so the specified
			ArrayFloat2		= 34,	// arrays must remain valid in memory while the shader runs.
			ArrayFloat3		= 35,
			ArrayFloat4		= 36,
			ArrayFloat9		= 41,
			ArrayFloat16	= 48,
			ArrayInt		= 64,
		};
	};

	float		mVal[16];
	byte		mType;
	const void*	mPtr;
	uint		mElements;
	
	Uniform() : mType(Type::Invalid), mPtr(0), mElements(0) {}

	Uniform (float				val) { *this = val; }
	Uniform (const Vector2f&	val) { *this = val; }
	Uniform (const Vector3f&	val) { *this = val; }
	Uniform (const Color4f&		val) { *this = val; }
	Uniform (const Quaternion&	val) { *this = val; }
	Uniform (const Matrix43&	val) { *this = val; }
	Uniform (const Matrix44&	val) { *this = val; }
	
	operator bool () const { return mType != Type::Invalid; }
	
	void operator = (float				v) { mType = Type::Float1;  mVal[0] = v; }
	void operator = (const Vector2f&	v) { mType = Type::Float2;  mVal[0] = v.x; mVal[1] = v.y; }
	void operator = (const Vector3f&	v) { mType = Type::Float3;  mVal[0] = v.x; mVal[1] = v.y; mVal[2] = v.z; }
	void operator = (const Color4f&		v) { mType = Type::Float4;  mVal[0] = v.r; mVal[1] = v.g; mVal[2] = v.b; mVal[3] = v.a; }
	void operator = (const Quaternion&	v) { mType = Type::Float4;  mVal[0] = v.x; mVal[1] = v.y; mVal[2] = v.z; mVal[3] = v.w; }
	void operator = (const Matrix43&	m) { mType = Type::Float16; for (uint i = 0; i < 16; ++i) mVal[i] = m[i]; }
	void operator = (const Matrix44&	m) { mType = Type::Float16; for (uint i = 0; i < 16; ++i) mVal[i] = m[i]; }
};