#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Generic variable type, similar to 'var' in JavaScript
//============================================================================================================

class Variable
{
private:

	#include "VariableMacros.h"

public:

	struct Type
	{
		enum
		{
			Invalid		= 0x00,
			Bool		= 0x01,		// 'true' or 'false'
			Int			= 0x02,		// 123 -- whole number, no dots or letters
			UInt		= 0x03,		// Same as Int, used for indices
			UShort		= 0x04,		// Same as Int
			Short2		= 0x05,		// 1 2 -- no dots
			Float		= 0x06,		// 123.456  -- contains a dot
			Float2		= 0x07,		// 1.2 3.4
			Float3		= 0x08,		// 1.2 3.4 5.6
			Float4		= 0x09,		// 1.2 3.4 5.6 7.8
			Color		= 0x0A,		// 0xRRGGBBAA
			String		= 0x0B,		// "text" (first and last letters are quotes)
			Array		= 0x10,
		};
	};

private:

	union
	{
		void*	mPtr;
		byte	mBytes[16];
	};

	byte	mType;

public:

	// Common type constructors
	explicit Variable()						: mType(Type::Invalid)	{ mPtr = 0; }
	explicit Variable(const char* val)		: mType(Type::String)	{ mPtr = new String(val); }
	explicit Variable(const Variable& val)	: mType(Type::Invalid)	{ *this = val; }
	
	// Destructor should always release currently used memory
	~Variable() { Release(); }

	// Releases the variable's data, clearing any used memory
	void Release();

	// If copying of memory is desired, this function will do just that
	void operator =(const Variable& val);

	// Whether the variable contains a valid value that can be saved
	bool IsValid() const;

	// Whether the variable contains an array
	bool IsArray() const { return (mType & Type::Array) != 0; }

	// Character array should behave like a string
	void operator =(const char* val) { ToString() = val; }

	// Common type implementations, defined in "VariableMacros.h"
	VAR_SMALL_IMPL(bool, Bool, Bool);
	VAR_SMALL_IMPL(int32, Int, Int);
	VAR_SMALL_IMPL(uint32, UInt, UInt);
	VAR_SMALL_IMPL(ushort, UShort, UShort);
	VAR_SMALL_IMPL(float, Float, Float);
	VAR_SMALL_IMPL(Vector2i, Vector2i, Short2);
	VAR_SMALL_IMPL(Vector2f, Vector2f, Float2);
	VAR_SMALL_IMPL(Vector3f, Vector3f, Float3);
	VAR_SMALL_IMPL(Quaternion, Quaternion, Float4);
	VAR_SMALL_IMPL(Color3f, Color3f, Float3);
	VAR_SMALL_IMPL(Color4f, Color4f, Float4);
	VAR_SMALL_IMPL(Color4ub, Color4ub, Color);

	// NOTE: Allocated implementations created with this macro must also
	// modify the destructor in order to ensure that they are deleted properly.
	VAR_LARGE_IMPL(String, String);

	// Convenience function: returns the string-encoded representation of the data
	String GetString() const;

public:

	// Serializes to the end of the specified memory buffer
	bool SerializeTo (Memory& mem) const;

	// Serializes from the specified buffer, updating the buffer and size values
	bool SerializeFrom (ConstBytePtr& buffer, uint& size);

	// Serializes from the specified memory buffer
	bool SerializeFrom (const Memory& mem)
	{
		ConstBytePtr buffer = mem.GetBuffer();
		uint size = mem.GetSize();
		return SerializeFrom(buffer, size);
	}

	// Serialization to and from string format
	bool SerializeTo (String& s, uint tabs = 0) const;
	bool SerializeFrom (const String& s);

public:

	// Convenience functions that serialize into the provided variable and return whether succeeded
	bool operator >> (bool& value) const;
	bool operator >> (int& value) const;
	bool operator >> (uint& value) const;
	bool operator >> (ushort& value) const;
	bool operator >> (float& value) const;
	bool operator >> (Vector2i& value) const;
	bool operator >> (Vector2f& value) const;
	bool operator >> (Vector3f& value) const;
	bool operator >> (Quaternion& value) const;
	bool operator >> (Color3f& value) const;
	bool operator >> (Color4f& value) const;
	bool operator >> (Color4ub& value) const;
	bool operator >> (String& value) const { value = (IsString() ? AsString() : GetString()); return true; }
};

//============================================================================================================
// Conversion functions
//============================================================================================================

inline void operator >> (bool	value,	Variable& var) { var.ToBool()	= value; }
inline void operator >> (int	value,	Variable& var) { var.ToInt()	= value; }
inline void operator >> (uint	value,	Variable& var) { var.ToUInt()	= value; }
inline void operator >> (ushort value,	Variable& var) { var.ToUShort()	= value; }
inline void operator >> (float	value,	Variable& var) { var.ToFloat()	= value; }

inline void operator >> (const Vector2i&	value,	Variable& var) { var.ToVector2i()	= value; }
inline void operator >> (const Vector2f&	value,	Variable& var) { var.ToVector2f()	= value; }
inline void operator >> (const Vector3f&	value,	Variable& var) { var.ToVector3f()	= value; }
inline void operator >> (const Quaternion&	value,	Variable& var) { var.ToQuaternion()	= value; }
inline void operator >> (const Color3f&		value,	Variable& var) { var.ToColor3f()	= value; }
inline void operator >> (const Color4f&		value,	Variable& var) { var.ToColor4f()	= value; }
inline void operator >> (const Color4ub&	value,	Variable& var) { var.ToColor4ub()	= value; }
inline void operator >> (const String&		value,	Variable& var) { var.ToString()		= value; }