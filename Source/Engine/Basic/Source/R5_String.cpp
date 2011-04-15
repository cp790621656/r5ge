#include "../Include/_All.h"
#include <stdarg.h>
using namespace R5;

//======================================================================================================

#ifndef _WINDOWS
	// Non-case sensitive string comparison
	#define _stricmp strcasecmp
#endif

//======================================================================================================
// Quick helper function returning lower-case of the specified char
//======================================================================================================

inline char LowerCase (char c)
{
	return (c < 91 && c > 64) ? c + 32 : c;
}

//======================================================================================================
// helper function that checks to see whether the specified character is a numeric one
//======================================================================================================

inline bool IsNumeric (char c)
{
	if (c < '0') return false;
	if (c < ':') return true;
	return false;
}

//======================================================================================================
// Helper function that's used to determine where the words begin and end
//======================================================================================================

inline bool IsWordChar (char c)
{
	if (c < '!') return false;
	if (c < '~') return true;
	return false;
}

//======================================================================================================
// Constructor can act as String::Format
//======================================================================================================

String::String(const char* format, ...) : mLarge(0), mLength(0), mAllocated(0)
{
	mSmall[0] = 0;

	if (format != 0 && format[0] != 0)
	{
		va_list args;
		va_start( args, format );

#ifdef _WINDOWS
		uint length = _vscprintf(format, args);
		
		if (length > 0)
		{
			Resize(length);
			vsprintf( GetBuffer(), format, args );
		}
#else
		char tmp[256] = {0};
		vsprintf( tmp, format, args );

		uint length = strlen(tmp);

		if (length != 0 && length < 256)
		{
			*this = tmp;
		}
		else
		{
			ASSERT(false, "Format string must evaluate to no more than 256 characters!");
		}
#endif
		va_end( args );
	}
}

//======================================================================================================
// Debug mode destructor
//======================================================================================================

#ifdef _STRING_DEBUG_MODE_
String::~String()
{
	if (mLarge != 0)
	{
		System::Log("String -- DESTRUCTOR ('%s') - %u bytes deallocated", mLarge, mAllocated);
		delete [] mLarge;
		mLarge = 0;
	}
	else if (mSmall[0] != 0)
	{
		System::Log("String -- DESTRUCTOR ('%s') - no memory was allocated", mSmall);
	}
}
#endif

//======================================================================================================
// Comparison operator for sorting
//======================================================================================================

bool String::operator < (const String& txt) const
{
	return (strcmp(GetBuffer(), txt.GetBuffer()) < 0);
}

//======================================================================================================
// Appends one string to another. Example: String("Hello ") << "world" << "!" == "Hello world!"
//======================================================================================================

String& String::operator << (const char* txt)
{
	if (txt && txt[0])
	{
		uint newLength = mLength + (uint)strlen(txt);
		Reserve(newLength);
		strcpy(GetBuffer() + mLength, txt);
		mLength = newLength;
	}
	return *this;
}

//======================================================================================================

String& String::operator << (const String& txt)
{
	if (txt.IsValid())
	{
		uint newLength = mLength + txt.GetLength();
		Reserve(newLength);
		strcpy(GetBuffer() + mLength, txt.GetBuffer());
		mLength = newLength;
	}
	return *this;
}

//======================================================================================================
// Basic assignment operators
//======================================================================================================

String& String::operator = (const char* txt)
{
	Clear();

	if (txt && txt[0])
	{
		uint newLength = (uint)strlen(txt);
		Reserve(newLength);
		strcpy(GetBuffer(), txt);
		mLength = newLength;
	}

	return *this;
}

//======================================================================================================

String& String::operator = (const String& txt)
{
	if (&txt != this)
	{
		Clear();

		if (txt.IsValid())
		{
			mLength = txt.mLength;
			Reserve(mLength);
			strcpy(GetBuffer(), txt.GetBuffer());
		}
	}
	return *this;
}

//======================================================================================================
// Basic equality check
//======================================================================================================

bool String::operator == (const char* txt) const
{
	// If buffer is empty
	if ( IsEmpty() )
	{
		// If the text is also empty, it's a match
		return ( txt == 0 || txt[0] == 0 );
	}
	// If the buffer is not empty, but the text is, it's clearly not a match
	else if ( txt == 0 || txt[0] == 0 ) return false;

	// Compare normally
	return (_stricmp(GetBuffer(), txt) == 0);
}

//==========================================================================================================
// Whether the string contains an integer value
//==========================================================================================================

bool String::IsInt() const
{
	if (IsEmpty()) return false;
	const char* buffer = GetBuffer();

	// First character can be a negative sign
	if (buffer[0] != '-' && !::IsNumeric(buffer[0])) return false;

	// Following characters must be numeric
	for (uint i = 1; i < mLength; ++i)
	{
		// If it's not a numeric character, then neither is the string
		if (!::IsNumeric(buffer[i])) return false;
	}
	return true;
}

//==========================================================================================================
// Whether the string contains a floating-point value
//==========================================================================================================

bool String::IsFloat() const
{
	if (IsEmpty()) return false;
	const char* buffer = GetBuffer();

	uint dots = 0;
	uint sign = 0;

	// First character can be a negative sign
	if (buffer[0] != '-' && !::IsNumeric(buffer[0])) return false;

	// Following characters must be numeric
	for (uint i = 1; i < mLength; ++i)
	{
		char ch = buffer[i];

		// Only one dot is allowed
		if (ch == '.')
		{
			if (dots++ == 0) continue;
			return false;
		}

		// Only one followup sign is allowed, followed by 'e'
		if (ch == '-' || ch == '+')
		{
			if (sign++ == 0)
			{			
				if (++i < mLength)
				{
					if (buffer[i] == 'e') continue;
				}
			}
			return false;
		}

		// If it's not a numeric character, then neither is the string
		if (!::IsNumeric(ch)) return false;
	}
	return true;
}

//==========================================================================================================
// Strips out any special characters found in the string, such as color encoding
//==========================================================================================================

void String::StripTags()
{
	byte ch;
	const char* buff (GetBuffer());

	for (uint i = 0, end = GetLength(); i < end; )
	{
		ch = buff[i];

		if ( (ch == '[') && (i + 7 < end) && (buff[i + 7] == ']') )
		{
			Erase(i, i + 8);
			end = GetLength();
		}
		else ++i;
	}
}

//======================================================================================================
// Finds where 'txt' begins in the string, returning the index. (retval == mLength) if not found.
//======================================================================================================

uint String::Find (const String&	match,
						   bool		caseSensitive,
						   uint		from,
						   uint		to,
						   bool		reverse) const
{
	if (match.IsValid())
	{
		if (to > mLength) to = mLength;

		uint last (match.GetLength() - 1);
		const char*	 txt  (match.GetBuffer());

		if (from + last < mLength)
		{
			const char* buff	( GetBuffer() );
			const char* search	( buff + from );
			const char* end		( buff + to - last );

			if (reverse)
			{
				if (caseSensitive)
				{
					// Start at the end and move toward the front
					for (const char* offset = end - 1; (offset >= search); --offset)
						for (uint i = 0; (offset[i] == txt[i]); ++i)
							if (i == last) return (uint)(offset - buff);
				}
				else
				{
					// Start at the end and move toward the front
					for (const char* offset = end - 1; (offset >= search); --offset)
						for (uint i = 0; (LowerCase(offset[i]) == LowerCase(txt[i])); ++i)
							if (i == last) return (uint)(offset - buff);
				}
			}
			else
			{
				if (caseSensitive)
				{
					// Start at the beginning and move toward the end
					for (; (search < end) && (*search > 0); ++search)
						for (uint i = 0; (search[i] == txt[i]); ++i)
							if (i == last) return (uint)(search - buff);
				}
				else
				{
					// Start at the beginning and move toward the end
					for (; (search < end) && (*search > 0); ++search)
						for (uint i = 0; (LowerCase(search[i]) == LowerCase(txt[i])); ++i)
							if (i == last) return (uint)(search - buff);
				}
			}
		}
	}
	return mLength;
}

//======================================================================================================
// Replaces all instances of text with the replacement string
//======================================================================================================

uint String::Replace( const String& match, const String& replacement, bool caseSensitive )
{
	uint count (0);

	if ( match.IsValid() && replacement.IsValid() )
	{
		uint length (match.GetLength()), start(0);
		String out, temp;

		for ( ; ; )
		{
			uint pos = Find(match, caseSensitive, start);

			if (pos != GetLength())
			{
				if (GetString(temp, start, pos)) out << temp;
				out << replacement;
				start = pos + length;
				++count;
			}
			else
			{
				if (GetString(temp, start)) out << temp;
				break;
			}
		}

		if (count)
		{
			(*this) = out;
		}
	}
	return count;
}

//======================================================================================================
// Ensures that the string has enough room to hold newSize number of characters
//======================================================================================================

void String::Reserve(uint newSize)
{
	if (newSize != 0)
	{
		// Account for the end-of-string character
		++newSize;

		if (newSize > mAllocated && newSize > R5_SMALL_STRING_SIZE)
		{
			uint newAlloc = ((newSize > mAllocated * 2) ? newSize : mAllocated * 2);

			char* newBuffer = new char[newAlloc];

			if (IsValid())
			{
				strcpy(newBuffer, GetBuffer());
			}
			else newBuffer[0] = 0;

			if (mLarge != 0)
			{
				delete [] mLarge;
			}

			mLarge = newBuffer;
			mAllocated = newAlloc;
		}
	}
}

//======================================================================================================
// Resizes the string buffer to be of the specified size. Note that its contents are undefined.
//======================================================================================================

char* String::Resize (uint newSize)
{
	Reserve(newSize);
	char* buffer = GetBuffer();
	buffer[mLength = newSize] = 0;
	return buffer;
}

//======================================================================================================
// Same as "operator =", but with printf()-like functionality
//======================================================================================================

String& String::Set(const char* format, ...)
{
	Clear();

	if (format)
	{
		va_list args;
		va_start( args, format );

#ifdef _WINDOWS
		Reserve( mLength = _vscprintf(format, args) );
		vsprintf( GetBuffer(), format, args );
#else
		char tmp[512] = {0};
		vsnprintf( tmp, 511, format, args );
		mLength = strlen(tmp);

		if ( mLength != 0 && mLength < 512 )
		{
			Reserve(mLength);
			strcpy(GetBuffer(), tmp);
		}
		//else
		//{
		//	ASSERT(false, "Format string must evaluate to no more than 512 characters!");
		//}
#endif
		va_end( args );
	}

	return *this;
}

//======================================================================================================
// Appendable printf()-like functionality.   Example: String("Test").Append("%u", 123) == "Test123"
// Append() is a faster version of doing:             String("Test") << String("%u", 123)
//======================================================================================================

String& String::Append(const char* format, ...)
{
	if (format)
	{
		va_list args;
		va_start( args, format );

#ifdef _WINDOWS
		uint newLength = mLength + _vscprintf(format, args);
		Reserve( newLength );
		vsprintf( GetBuffer() + mLength, format, args );
		mLength = newLength;
#else
		char tmp[512] = {0};
		vsnprintf( tmp, 511, format, args );
		uint tmpLength = strlen(tmp);

		if ( tmpLength != 0 && tmpLength < 512 )
		{
			Reserve(mLength + tmpLength);
			strcpy(GetBuffer() + mLength, tmp);
			mLength += tmpLength;
		}
#endif
		va_end( args );
	}

	return *this;
}

//======================================================================================================
// Returns whether the string begins with the specified text
//======================================================================================================

bool String::BeginsWith (const char* text) const
{
	if (IsEmpty() || text == 0 || text[0] == 0) return false;

	const char* buffer = GetBuffer();
	uint i = 0;

	for ( ; i < mLength; ++i )
	{
		// String ends -- if we got here then the strings match
		if (text[i] == 0) return true;

		// If the letters don't match, we're done
		if (LowerCase(text[i]) != LowerCase(buffer[i]))
			return false;
	}

	// Only a match if both strings end here
	return (text[i] == 0);
}

//======================================================================================================
// Returns whether the string begins with the specified text
//======================================================================================================

bool String::EndsWith (const char* text) const
{
	if (IsEmpty() || text == 0 || text[0] == 0) return false;

	uint len = (uint)strlen(text);

	if (mLength < len) return false;
	const char* buffer = GetBuffer();
	
	for (uint i = mLength - len; i < mLength; ++i, ++text)
	{
		if (LowerCase(*text) != LowerCase(buffer[i]))
			return false;
	}
	return true;
}

//======================================================================================================
// Counts the number of times the specified character is encountered in the string
//======================================================================================================

uint String::Count (char letter) const
{
	uint count = 0;
	
	if (IsValid())
	{
		const char* buffer = GetBuffer();

		for (uint i = 0; i < mLength; ++i)
		{
			if (buffer[i] == letter) ++count;
		}
	}
	return count;
}

//======================================================================================================
// Retrieves a substring from the string
//======================================================================================================

bool String::GetString (String& out, uint from, uint to) const
{
	out.Clear();

	if (from > to)
	{
		uint temp = to;
		to = from;
		from = temp;
	}

	if (to > mLength) to = mLength;

	if (from < to)
	{
		out.mLength = to - from;
		out.Reserve(out.mLength);
		strncpy(out, GetBuffer() + from, out.mLength);
		out[out.mLength] = 0;
		return true;
	}
	return false;
}

//======================================================================================================
// Retrieves a substring from the string, trimmed at the front and back
//======================================================================================================

bool String::GetTrimmed (String& out, uint from, uint to) const
{
	out.Clear();

	if (from > to)
	{
		uint temp = to;
		to = from;
		from = temp;
	}

	if (to > mLength) to = mLength;
	const char* buffer = GetBuffer();

	// Skip starting spaces
	while (from < to && buffer[from] < 33) ++from;

	// Skip trailing spaces
	while (from < to && (buffer[to - 1] < 33)) --to;

	if (from < to)
	{
		out.mLength = to - from;
		out.Reserve(out.mLength);
		strncpy(out, buffer + from, out.mLength);
		out[out.mLength] = 0;
		return true;
	}
	return false;
}

//======================================================================================================
// Retrieves a continuous line at the specified coordinates, up to the last word that fits
//======================================================================================================

uint String::GetPhrase (String& out, uint from, uint to) const
{
	if (to < mLength)
	{
		// Backtrack to the last space
		uint index = to;
		while (index > from && (*this)[index] > 32) --index;

		// Found a space -- the line will count up to this character, skipping the space
		if (index > from)
		{
			GetString(out, from, index);
			return index + 1;
		}
	}

	// No modification to the line -- remember it as-is
	GetString(out, from, to);
	return to;
}

//======================================================================================================
// Retrieves a single continuous line at the specified coordinates
//======================================================================================================

uint String::GetLine (String& out, uint from, uint to) const
{
	out.Clear();

	if (from > to)
	{
		uint temp = to;
		to = from;
		from = temp;
	}

	if (to > mLength) to = mLength;

	const char* buffer = GetBuffer();

	// Skip starting spaces
	while ( from < to && !::IsWordChar(buffer[from]) )
	{
		if (buffer[from] == 0) return mLength;
		++from;
	}

	// Find the end of the line
	for (uint i = from + 1; i < to; ++i)
	{
		if ( buffer[i] == '\n' || buffer[i] == '\r' )
		{
			to = i;
			break;
		}
	}

	// Skip trailing spaces
	while (from < to && (buffer[to - 1] < 33)) --to;

	// If there is something left, save it out
	if (from < to)
	{
		out.mLength = to - from;
		out.Reserve(out.mLength);
		strncpy(out, buffer + from, out.mLength);
		out[out.mLength] = 0;
		return to;
	}
	return mLength;
}

//======================================================================================================
// Retrieves a single continuous word at the specified coordinates
//======================================================================================================

uint String::GetWord (String& out, uint from, uint to) const
{
	out.Clear();

	if (from > to)
	{
		uint temp = to;
		to = from;
		from = temp;
	}

	if (to > mLength) to = mLength;

	const char* buffer = GetBuffer();

	// Skip starting spaces
	while ( from < to && !::IsWordChar(buffer[from]) )
	{
		if (buffer[from] == 0) return mLength;
		++from;
	}

	// Find the end of the word
	for (uint i = from + 1; i < to; ++i)
	{
		if ( !::IsWordChar(buffer[i]) )
		{
			to = i;
			break;
		}
	}

	// If there is something left, save it out
	if (from < to)
	{
		out.mLength = to - from;
		out.Reserve(out.mLength);
		strncpy(out, buffer + from, out.mLength);
		out[out.mLength] = 0;
		return to;
	}
	return mLength;
}

//======================================================================================================
// Splits the string on the given character, copying each respective side into a string of their own
//======================================================================================================

bool String::Split ( String& left, char splitChar, String& right ) const
{
	if (mLength > 0)
	{
		const char* buff (GetBuffer());

		for (uint i = 0; i < mLength; ++i)
		{
			char ch = buff[i];

			if (ch == splitChar)
			{
				GetString(left, 0, i);
				GetString(right, i+1);
				return true;
			}
		}
	}
	return false;
}

//======================================================================================================
// Erases the specified range of characters from the string
//======================================================================================================

bool String::Erase (uint from, uint to)
{
	if (to > mLength) to = mLength;

	if (from < to)
	{
		uint tail = mLength - to;
		char* buff (GetBuffer());
		memmove(buff + from, buff + to, tail);
		buff[from + tail] = 0;
		mLength -= (to - from);
		return true;
	}
	return false;
}

//======================================================================================================
// Fully loads a file into the string
//======================================================================================================

bool String::Load (const char* filename)
{
	Memory mem;

	if (mem.Load(filename))
	{
		mLength = mem.GetSize();
		Reserve(mLength);
		char* buff = GetBuffer();
		memcpy(buff, mem.GetBuffer(), mLength);
		buff[mLength] = 0;
	}
	return true;
}

//======================================================================================================
// Writes the string's data into the specified filename
//======================================================================================================

bool String::Save (const char* filename) const
{
	if (IsValid())
	{
		FILE* fp = fopen(filename, "wb");
		if (!fp) return false;

		fwrite(GetBuffer(), 1, mLength, fp);

		fclose(fp);
		return true;
	}
	return false;
}

//======================================================================================================
// Removes trailing zeros in a floating-point value so 1.230000 becomes 1.23
//======================================================================================================

void String::TrimFloat()
{
	if (mLength > 1)
	{
		char* buffer = (mLarge == 0) ? mSmall : mLarge;
		char ch = buffer[mLength-1];

		while (mLength > 1)
		{
			if (ch != '0') break;
			ch = buffer[mLength-2];
			if (ch < '0' || ch > '9') break;
			buffer[--mLength] = 0;
		}
	}
}

//======================================================================================================
// Returns a formatted number in ###,###,### format
//======================================================================================================

String String::GetFormattedSize (ulong size)
{
	ulong hundred  = size % 1000;
	ulong thousand = (size / 1000) % 1000;
	ulong million  = (size / 1000000);

	String text;

	if		( million  > 0 )	text.Set("%u,%3d,%3d", million, thousand, hundred);
	else if ( thousand > 0 )	text.Set("%u,%3d", thousand, hundred);
	else						text.Set("%u", hundred);

	for (uint i = 0; i < text.GetLength(); ++i)
		if (text[i] == ' ')
			text[i] =  '0';

	return text;
}

//======================================================================================================
// Conversion functions
//======================================================================================================

bool String::operator >> (bool& value) const
{
	value = (*this == "1" || *this == "true");
	return true;
}

//======================================================================================================

bool String::operator >> (char& value) const
{
	int temp;
	if (*this >> temp)
	{
		value = (char)temp;
		return true;
	}
	return false;
}

//======================================================================================================

bool String::operator >> (byte& value) const
{
	uint temp;
	if (*this >> temp)
	{
		value = (byte)temp;
		return true;
	}
	return false;
}

//======================================================================================================

bool String::operator >> (short& value) const
{
	int temp;
	if (*this >> temp)
	{
		value = (short)temp;
		return true;
	}
	return false;
}

//======================================================================================================

bool String::operator >> (ushort& value) const
{
	uint temp;
	if (*this >> temp)
	{
		value = (ushort)temp;
		return true;
	}
	return false;
}

//======================================================================================================

bool String::operator >> (int& value) const
{
	return IsValid() && (sscanf(GetBuffer(), "%d", &value) == 1);
}

//======================================================================================================

bool String::operator >> (uint& value) const
{
	return IsValid() && (sscanf(GetBuffer(), "%u", &value) == 1);
}

//======================================================================================================

bool String::operator >> (float& value) const
{
	return IsValid() && (sscanf(GetBuffer(), "%f", &value) == 1);
}

//======================================================================================================

bool String::operator >> (double& value) const
{
	return IsValid() && (sscanf(GetBuffer(), "%lf", &value) == 1);
}
