#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

//#define _STRING_DEBUG_MODE_
#define _CRT_SECURE_NO_DEPRECATE

//============================================================================================================
// The string should be 64-byte aligned
//============================================================================================================

#if defined(__x86_64__) || defined(__amd64__) || defined(__AMD64__)
 #define R5_SMALL_STRING_SIZE 40
#else
 #define R5_SMALL_STRING_SIZE 52
#endif

//============================================================================================================
// High-performance string class
//============================================================================================================

class String
{
	char	mSmall[R5_SMALL_STRING_SIZE];	// The initial buffer should fit most strings
	char*	mLarge;							// Failing that, a larger buffer is allocated
	uint	mLength;						// Current length of the text
	uint	mAllocated;						// Number of bytes allocated for the 'mLarge' buffer

public:

	String()				: mLarge(0), mLength(0), mAllocated(0) { mSmall[0] = 0; }
	String(const String& s)	: mLarge(0), mLength(0), mAllocated(0) { mSmall[0] = 0; *this = s; }
	String(uint reserve)	: mLarge(0), mLength(0), mAllocated(0) { mSmall[0] = 0; Reserve(reserve); }
	String(const char* s, ...);

#ifdef _STRING_DEBUG_MODE_
	~String();
#else
	~String() { if (mLarge != 0) { delete [] mLarge; mLarge = 0; } }
#endif

	char&		operator [] (uint i)					{ return (mLarge == 0) ? mSmall[i] : mLarge[i]; }
	const char	operator [] (uint i)	const			{ return (mLarge == 0) ? mSmall[i] : mLarge[i]; }

	char&		operator [] (int i)						{ return (mLarge == 0) ? mSmall[i] : mLarge[i]; }
	const char	operator [] (int i)	const				{ return (mLarge == 0) ? mSmall[i] : mLarge[i]; }

	bool		operator <  (const String& txt) const;
	String&		operator << (const char* txt);
	String&		operator << (const String& in);
	bool		operator >> (const char* file) const	{ return Save(file); }
	void		operator >> (String& txt) const			{ txt = *this; }

	String&		operator << (bool val)					{ return *this << (val ? "true" : "false"); }
	String&		operator << (char val)					{ return Append("%d", val); }
	String&		operator << (short val)					{ return Append("%d", val); }
	String&		operator << (int val)					{ return Append("%d", val); }
	String&		operator << (byte val)					{ return Append("%u", val); }
	String&		operator << (ushort val)				{ return Append("%u", val); }
	String&		operator << (uint val)					{ return Append("%u", val); }
	String&		operator << (float val)					{ Append("%f", val); TrimFloat(); return *this; }
	String&		operator << (double val)				{ Append("%f", val); TrimFloat(); return *this; }

	bool		operator >> (bool&		value) const;
	bool		operator >> (char&		value) const;
	bool		operator >> (byte&		value) const;
	bool		operator >> (short&		value) const;
	bool		operator >> (ushort&	value) const;
	bool		operator >> (int&		value) const;
	bool		operator >> (uint&		value) const;
	bool		operator >> (float&		value) const;
	bool		operator >> (double&	value) const;

	String&		operator  = (const char* txt);
	String&		operator  = (const String& in);
	String&		operator  = (bool val)					{ return *this = (val ? "true" : "false"); }
	String&		operator  = (char value)				{ return Set("%d", value); }
	String&		operator  = (short value)				{ return Set("%d", value); }
	String&		operator  = (int value)					{ return Set("%d", value); }
	String&		operator  = (byte value)				{ return Set("%u", value); }
	String&		operator  = (ushort value)				{ return Set("%u", value); }
	String&		operator  = (uint value)				{ return Set("%u", value); }
	String&		operator  = (float value)				{ return Set("%f", value); }
	String&		operator  = (double value)				{ return Set("%lf", value); }

	bool		operator == (const char* txt)	const;
	bool		operator != (const char* txt)	const	{ return !(*this == txt); }
	bool		operator == (const String& txt) const	{ return (mLength == txt.mLength && *this == txt.GetBuffer()); }
	bool		operator != (const String& txt) const	{ return (mLength != txt.mLength || *this != txt.GetBuffer()); }
				operator	const char*()		const	{ return (mLarge == 0) ? mSmall : mLarge; }
				operator	char*()						{ return (mLarge == 0) ? mSmall : mLarge; }
				operator	bool()				const	{ return (mLength != 0) && (mLarge == 0 ? (mSmall[0] != 0) : (mLarge[0] != 0)); }

	char*			GetBuffer()				{ return (mLarge == 0) ? mSmall : mLarge; }
	const char*		GetBuffer()		const	{ return (mLarge == 0) ? mSmall : mLarge; }
	bool			IsValid()		const	{ return (mLength != 0) && (mLarge == 0 ? (mSmall[0] != 0) : (mLarge[0] != 0)); }
	bool			IsEmpty()		const	{ return (mLength == 0) || (mLarge == 0 ? (mSmall[0] == 0) : (mLarge[0] == 0)); }
	bool			IsInt()			const;
	bool			IsFloat()		const;
	uint			GetSize()		const	{ return mLength; } // For the FOREACH macro
	uint			GetLength()		const	{ return mLength; }
	uint			GetAllocated()	const	{ return mAllocated; }
	char&			Expand()				{ Resize(mLength + 1); char* buff = GetBuffer() + mLength; *buff = 0; return *(buff - 1); }
	void			StripTags();

	uint	Find		( const String& phrase, bool caseSensitive = true, uint from = 0, uint to = 0xFFFFFFFF, bool reverse = false ) const;
	uint	Replace		( const String& match, const String& replacement, bool caseSensitive = false );
	bool	Contains	( const String& phrase ) const { return (Find(phrase, false) != mLength); }

	void	Reserve		( uint newSize );
	char*	Resize		( uint newSize );
	String&	Set			( const char* format, ... );
	String&	Append		( const char* format, ... );
	bool	BeginsWith	( const char* text ) const;
	bool	EndsWith	( const char* text ) const;
	bool	Contains	( const char* text, bool caseSensitive = false ) const { return Find(text, caseSensitive) != mLength; }
	uint	Count		( char letter ) const;
	bool	GetString	( String& out, uint from = 0, uint to = 0xFFFFFFFF ) const;
	bool	GetTrimmed	( String& out, uint from = 0, uint to = 0xFFFFFFFF ) const;
	uint	GetPhrase	( String& out, uint from = 0, uint to = 0xFFFFFFFF ) const;
	uint	GetLine		( String& out, uint from = 0, uint to = 0xFFFFFFFF ) const;
	uint	GetWord		( String& out, uint from = 0, uint to = 0xFFFFFFFF ) const;
	bool	Split		( String& left, char splitChar, String& right ) const;
	bool	Erase		( uint from = 0, uint to = 0xFFFFFFFF );
	bool	Load		( const char* filename );
	bool	Save		( const char* filename ) const;

	void Clear()
	{
		if (mLarge != 0) mLarge[0] = 0;
		mSmall[0] = 0;
		mLength = 0;
	}

	void Release()
	{
		if (mLarge != 0)
		{
			delete [] mLarge;
			mLarge = 0;
			mAllocated = 0;
		}
		mSmall[0] = 0;
		mLength = 0;
	}

	// Removes trailing zeros in a floating-point value so 1.230000 becomes 1.23
	void TrimFloat();

	// Returns a formatted number in ###,###,### format
	static String GetFormattedSize(ulong size);
};

inline String& operator >> (const char*	value, String& str)	{ return str = value; }
inline String& operator >> (bool		value, String& str)	{ return str = (value ? "true" : "false"); }
inline String& operator >> (char		value, String& str)	{ return str.Set("%d", value); }
inline String& operator >> (short		value, String& str)	{ return str.Set("%d", value); }
inline String& operator >> (int			value, String& str)	{ return str.Set("%d", value); }
inline String& operator >> (byte		value, String& str)	{ return str.Set("%u", value); }
inline String& operator >> (ushort		value, String& str)	{ return str.Set("%u", value); }
inline String& operator >> (uint		value, String& str)	{ return str.Set("%u", value); }
inline String& operator >> (float		value, String& str)	{ return str.Set("%f", value); }
inline String& operator >> (double		value, String& str)	{ return str.Set("%f", value); }

inline String operator + (const char* left, String& right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (const char* left, const String& right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (String& left, String& right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (String& left, const String& right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (const String& left, String& right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (const String& left, const String& right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (const String& left, const char* right)
{
	String out;
	out << left;
	out << right;
	return out;
}

inline String operator + (String& left, const char* right)
{
	String out;
	out << left;
	out << right;
	return out;
}
