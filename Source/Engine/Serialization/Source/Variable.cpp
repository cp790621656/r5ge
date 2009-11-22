#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Helper macros used by serialization functions in order to shorten the amount of code
//============================================================================================================

#define VAR_ARRAY_PTR(varType)	((Array<varType>*)mPtr)

//============================================================================================================

#define TO_STRING_ARRAY_CASE(varType, funcName, typeName)		\
	case Type::typeName:										\
	{															\
		const Array<varType>& a = As##funcName##Array();		\
																\
		if (a.GetSize() > 0)									\
		{														\
			for (uint i = 0; i < a.GetSize(); ++i)				\
			{													\
				if (tabs > 0) s << tabString;					\
				s << "\t";										\
				s << a[i];										\
				s << "\n";										\
			}													\
		}														\
	}															\
	break;

//============================================================================================================

#define PARSE_MULTIPLE_CASE(varType, funcName, typeName)		\
	case Type::typeName:										\
	{															\
		Array<varType>& a = To##funcName##Array();				\
																\
		for (uint i = 0, imax = line.GetLength(); i < imax;)	\
		{														\
			i = line.GetWord(word, i);							\
			if (word.IsValid()) word >> a.Expand();				\
		}														\
	}															\
	break;

//============================================================================================================

#define SAVE_MULTIPLE_CASE(varType, funcName, typeName)			\
	case Type::typeName:										\
	{															\
		const Array<varType>& a = As##funcName##Array();		\
		uint size = a.GetSize();								\
																\
		for (uint i = 0; i < size; )							\
		{														\
			if ((i & 0xF) == 0)									\
			{													\
				if (tabs > 0) s << tabString;					\
				s << "\t";										\
			}													\
			else s << " ";										\
																\
			s << a[i];											\
																\
			if ((++i & 0xF) == 0 || i == size) s << "\n";		\
		}														\
	}															\
	break;

//============================================================================================================
// Helper function that determines whether the string contains only non-zero numbers (indices)
//============================================================================================================

inline bool ContainsIndices (const String& s)
{
	for (uint i = 0, imax = s.GetLength(); i < imax; ++i)
	{
		char ch = s[i];

		if (ch == ' ' || ch == '\t') continue;
		if (ch < '0' || ch > '9') return false;
	}
	return true;
}

//============================================================================================================
// Parse the specified string into the variable
//============================================================================================================

uint Parse (const String& s, Variable* var)
{
	uint length = s.GetLength();

	if (length > 0)
	{
		// Strings begin and end with quotes
		if (s[0] == '"' && s[length - 1] == '"')
		{
			if (length > 2)
			{
				if (var)
				{
					String& text = var->ToString();
					char* buffer = text.Resize(length - 2);
					memcpy(buffer, (s.GetBuffer() + 1), length - 2);
				}
				return Variable::Type::String;
			}
		}
		else
		{
			// Count the number of spaces and periods as they are clues as to what type of data we're dealing with
			uint spaces  = s.Count(' ');
			uint periods = s.Count('.');

			// No spaces or periods
			if (spaces == 0)
			{
				if (periods == 0)
				{
					// Booleans are either 'true' or 'false'
					if (s == "true")
					{
						if (var) *var = true;
						return Variable::Type::Bool;
					}
					
					if (s == "false")
					{
						if (var) *var = false;
						return Variable::Type::Bool;
					}

					// Colors come in 0xRRGGBB and 0xRRGGBBAA formats
					if (length > 7 && s[0] == '0' && s[1] == 'x')
					{
						if (length > 9)
						{
							Color4ub color;

							if (s >> color)
							{
								if (var) *var = color;
								return Variable::Type::Color;
							}
						}
						else
						{
							Color3f color;

							if (s >> color)
							{
								if (var) *var = color;
								return Variable::Type::Float3;
							}
						}
					}

					// Try to load the string as an integer
					int val;
						
					if (s >> val)
					{
						if (var) *var = (int32)val;
						return Variable::Type::Int;
					}
				}
				else if (periods == 1)
				{
					// Try to load the string as a floating point value
					float val;

					if (s >> val)
					{
						if (var) *var = val;
						return Variable::Type::Float;
					}
				}
			}
			else if (spaces == 1)
			{
				if (periods == 0)
				{
					Vector2i val;

					if (s >> val)
					{
						if (var) *var = val;
						return Variable::Type::Short2;
					}
				}
				else
				{
					Vector2f val;

					if (s >> val)
					{
						if (var) *var = val;
						return Variable::Type::Float2;
					}
				}
			}
			else if (spaces == 2)
			{
				Vector3f val;

				if (s >> val)
				{
					if (var) *var = val;
					return Variable::Type::Float3;
				}
			}
			else if (spaces == 3)
			{
				Quaternion val;

				if (s >> val)
				{
					if (var) *var = val;
					return true;
				}
			}
			else if (ContainsIndices(s))
			{
				// Unspecified set of indices is always of 'ushort' type
				ushort val;

				if (s >> val)
				{
					if (var) *var = val;
					return Variable::Type::UShort;
				}
			}
		}

		// Exact type could not be determined -- just treat it as a string
		if (var) *var = s;
		return Variable::Type::String;
	}
	return Variable::Type::Invalid;
}

//============================================================================================================
// Releases the variable's data, clearing any used memory
//============================================================================================================

void Variable::Release()
{
	if (IsArray())
	{
		switch (mType & 0xF)
		{
			case Type::Bool:		delete VAR_ARRAY_PTR(bool);			break;
			case Type::Int:			delete VAR_ARRAY_PTR(int32);		break;
			case Type::UInt:		delete VAR_ARRAY_PTR(uint32);		break;
			case Type::UShort:		delete VAR_ARRAY_PTR(ushort);		break;
			case Type::Float:		delete VAR_ARRAY_PTR(float);		break;
			case Type::Short2:		delete VAR_ARRAY_PTR(Vector2i);		break;
			case Type::Float2:		delete VAR_ARRAY_PTR(Vector2f);		break;
			case Type::Float3:		delete VAR_ARRAY_PTR(Vector3f);		break;
			case Type::Float4:		delete VAR_ARRAY_PTR(Quaternion);	break;
			case Type::Color:		delete VAR_ARRAY_PTR(Color4ub);		break;
			case Type::String:		delete VAR_ARRAY_PTR(String);		break;
		}
	}
	else if (mType == Type::String)
	{
		delete ((String*)mPtr);
	}

	mType = Type::Invalid;
	mPtr  = 0;
}

//============================================================================================================
// If copying of memory is desired, this function will do just that
//============================================================================================================

void Variable::operator =(const Variable& val)
{
	byte type = (val.mType & 0xF);

	if (type != Type::Invalid)
	{
		if (val.IsArray())
		{
			switch (type)
			{
				case Type::Bool:		ToBoolArray()		= val.AsBoolArray();		break;
				case Type::Int:			ToIntArray()		= val.AsIntArray();			break;
				case Type::UInt:		ToUIntArray()		= val.AsUIntArray();		break;
				case Type::UShort:		ToUShortArray()		= val.AsUShortArray();		break;
				case Type::Short2:		ToVector2iArray()	= val.AsVector2iArray();	break;
				case Type::Float:		ToFloatArray()		= val.AsFloatArray();		break;
				case Type::Float2:		ToVector2fArray()	= val.AsVector2fArray();	break;
				case Type::Float3:		ToVector3fArray()	= val.AsVector3fArray();	break;
				case Type::Float4:		ToQuaternionArray() = val.AsQuaternionArray();	break;
				case Type::Color:		ToColor4ubArray()	= val.AsColor4ubArray();	break;
				case Type::String:		ToStringArray()		= val.AsStringArray();		break;
			}
		}
		else
		{
			switch (type)
			{
				case Type::Bool:		ToBool()		= val.AsBool();			break;
				case Type::Int:			ToInt()			= val.AsInt();			break;
				case Type::UInt:		ToUInt()		= val.AsUInt();			break;
				case Type::UShort:		ToUShort()		= val.AsUShort();		break;
				case Type::Short2:		ToVector2i()	= val.AsVector2i();		break;
				case Type::Float:		ToFloat()		= val.AsFloat();		break;
				case Type::Float2:		ToVector2f()	= val.AsVector2f();		break;
				case Type::Float3:		ToVector3f()	= val.AsVector3f();		break;
				case Type::Float4:		ToQuaternion()	= val.AsQuaternion();	break;
				case Type::Color:		ToColor4ub()	= val.AsColor4ub();		break;
				case Type::String:		ToString()		= val.AsString();		break;
			}
		}
	}
	else if (mType != Type::Invalid)
	{
		Release();
	}
}

//============================================================================================================
// Whether the variable contains a valid value that can be saved
//============================================================================================================

bool Variable::IsValid() const
{
	if (mType == Type::Invalid) return false;
	
	if (IsArray())
	{
		byte type = mType & 0xF;

		switch (type)
		{
			case Type::Bool:		return AsBoolArray().IsValid();
			case Type::Int:			return AsIntArray().IsValid();
			case Type::UInt:		return AsUIntArray().IsValid();
			case Type::UShort:		return AsUShortArray().IsValid();
			case Type::Short2:		return AsVector2iArray().IsValid();
			case Type::Float:		return AsFloatArray().IsValid();
			case Type::Float2:		return AsVector2fArray().IsValid();
			case Type::Float3:		return AsVector3fArray().IsValid();
			case Type::Float4:		return AsQuaternionArray().IsValid();
			case Type::Color:		return AsColor4ubArray().IsValid();
			case Type::String:		return AsStringArray().IsValid();
		}
	}
	else if (mType == Type::String)
	{
		return AsString().IsValid();
	}
	return true;
}

//============================================================================================================
// Conversion to String
//============================================================================================================

String Variable::GetString() const
{
	String value;
	{
		if		(IsString())		AsString()		>> value;
		else if (IsBool())			AsBool()		>> value;
		else if (IsInt())			(int)AsInt()	>> value;
		else if (IsUInt())			(uint)AsUInt()	>> value;
		else if (IsUShort())		AsUShort()		>> value;
		else if (IsFloat())			AsFloat()		>> value;
		else if (IsVector2i())		AsVector2i()	>> value;
		else if (IsVector2f())		AsVector2f()	>> value;
		else if (IsVector3f())		AsVector3f()	>> value;
		else if (IsQuaternion())	AsQuaternion()	>> value;
		else if (IsColor4ub())		AsColor4ub()	>> value;
	}
	return value;
}

//============================================================================================================
// Serializes to the end of the specified memory buffer
//============================================================================================================

bool Variable::SerializeTo (Memory& mem) const
{
	mem.Append(mType);

	if (mType != Type::Invalid)
	{
		if (IsArray())
		{
			switch (mType & 0xF)
			{
				case Type::Bool:	AsBoolArray().SerializeTo(mem);			break;
				case Type::Int:		AsIntArray().SerializeTo(mem);			break;
				case Type::UInt:	AsUIntArray().SerializeTo(mem);			break;
				case Type::UShort:	AsUShortArray().SerializeTo(mem);		break;
				case Type::Short2:	AsVector2iArray().SerializeTo(mem);		break;
				case Type::Float:	AsFloatArray().SerializeTo(mem);		break;
				case Type::Float2:	AsVector2fArray().SerializeTo(mem);		break;
				case Type::Float3:	AsVector3fArray().SerializeTo(mem);		break;
				case Type::Float4:	AsQuaternionArray().SerializeTo(mem);	break;
				case Type::Color:	AsColor4ubArray().SerializeTo(mem);		break;
				case Type::String:
				{
					// Save the current size of the buffer
					uint oldSize = mem.GetSize();

					// First reseve some space for the number of bytes used by this array
					mem.Expand(4);
					
					// Run through each string and append it
					const Array<String>& a = AsStringArray();
					for (uint i = 0, imax = a.GetSize(); i < imax; ++i) mem.Append(a[i]);

					// Update variable storing the amount of memory used by this array
					*(uint32*)(mem.GetBuffer() + oldSize) = (uint32)(mem.GetSize() - oldSize - 4);
				}
				break;
			}
		}
		else
		{
			switch (mType & 0xF)
			{
				case Type::Bool:	mem.Append(AsBool());		break;
				case Type::Int:		mem.Append(AsInt());		break;
				case Type::UInt:	mem.Append(AsUInt());		break;
				case Type::UShort:	mem.Append(AsUShort());		break;
				case Type::Short2:	mem.Append(AsVector2i());	break;
				case Type::Float:	mem.Append(AsFloat());		break;
				case Type::Float2:	mem.Append(AsVector2f());	break;
				case Type::Float3:	mem.Append(AsVector3f());	break;
				case Type::Float4:	mem.Append(AsQuaternion());	break;
				case Type::Color:	mem.Append(AsColor4ub());	break;
				case Type::String:	mem.Append(AsString());		break;
			}
		}
	}
	return true;
}

//============================================================================================================
// Serializes from the specified buffer, returning the number of bytes used by the variable
//============================================================================================================

bool Variable::SerializeFrom (ConstBytePtr& buffer, uint& size)
{
	if (size > 0)
	{
		byte type = *buffer;
		++buffer;
		--size;

		if (type == Type::Invalid)
		{
			if (mType != type) Release();
			return true;
		}

		if (size > 0)
		{
			if ((type & Type::Array) != 0)
			{
				switch (type & 0xF)
				{
					case Type::Bool:	return ToBoolArray().SerializeFrom(buffer, size);		break;
					case Type::Int:		return ToIntArray().SerializeFrom(buffer, size);		break;
					case Type::UInt:	return ToUIntArray().SerializeFrom(buffer, size);		break;
					case Type::UShort:	return ToUShortArray().SerializeFrom(buffer, size);		break;
					case Type::Short2:	return ToVector2iArray().SerializeFrom(buffer, size);	break;
					case Type::Float:	return ToFloatArray().SerializeFrom(buffer, size);		break;
					case Type::Float2:	return ToVector2fArray().SerializeFrom(buffer, size);	break;
					case Type::Float3:	return ToVector3fArray().SerializeFrom(buffer, size);	break;
					case Type::Float4:	return ToQuaternionArray().SerializeFrom(buffer, size);	break;
					case Type::Color:	return ToColor4ubArray().SerializeFrom(buffer, size);	break;
					case Type::String:
					{
						uint bytes;

						// Extract the number of bytes used by this array
						if (Memory::Extract(buffer, size, bytes))
						{
							// The number of bytes must not exceed the available memory
							if (bytes <= size)
							{
								// Save the starting number of bytes
								uint start = bytes;

								// Convert the variable to be a string array
								Array<String>& a = ToStringArray();
								a.Clear();

								// Extract strings one at a time. Note that the Expand() call
								// adds an entry to the array regardless of success.

								while (bytes > 0 && Memory::Extract(buffer, bytes, a.Expand())) {}

								// Adjust the size by the amount of bytes that were used up
								size -= start - bytes;
								
								// If the length of the array is more than 1, loading has been successful
								if (a.GetSize() > 1) return true;

								// Otherwise something has gone wrong
								ASSERT(false, "String array loading has failed!");
								a.Clear();
							}
						}
					}
					break;
				}
			}
			else
			{
				switch (type)
				{
					case Type::Bool:	return Memory::Extract(buffer, size, ToBool());
					case Type::Int:		return Memory::Extract(buffer, size, ToInt());
					case Type::UInt:	return Memory::Extract(buffer, size, ToUInt());
					case Type::UShort:	return Memory::Extract(buffer, size, ToUShort());
					case Type::Short2:	return Memory::Extract(buffer, size, ToVector2i());
					case Type::Float:	return Memory::Extract(buffer, size, ToFloat());
					case Type::Float2:	return Memory::Extract(buffer, size, ToVector2f());
					case Type::Float3:	return Memory::Extract(buffer, size, ToVector3f());
					case Type::Float4:	return Memory::Extract(buffer, size, ToQuaternion());
					case Type::Color:	return Memory::Extract(buffer, size, ToColor4ub());
					case Type::String:	return Memory::Extract(buffer, size, ToString());
				}
			}
		}
	}
	return false;
}

//============================================================================================================
// Serialization to string format
//============================================================================================================

bool Variable::SerializeTo (String& s, uint tabs) const
{
	if (IsValid())
	{
		if (IsArray())
		{
			String tabString;

			if (tabs > 0)
			{
				for (uint i = 0; i < tabs; ++i)
				{
					tabString << "\t";
				}
			}

			byte varType = mType & 0xF;

			// Expand the type of this array
			switch (varType)
			{
				case Type::Bool:	s << "Bool";		break;
				case Type::Int:		s << "Int";			break;
				case Type::UInt:	s << "UInt";		break;
				case Type::UShort:	s << "UShort";		break;
				case Type::Short2:	s << "Short2";		break;
				case Type::Float:	s << "Float";		break;
				case Type::Float2:	s << "Float2";		break;
				case Type::Float3:	s << "Float3";		break;
				case Type::Float4:	s << "Float4";		break;
				case Type::Color:	s << "Color";		break;
				default:			s << "String";		break;
			}

			// Expand the array indicator
			s << "[]\n";

			// Expand the tabs and the opening bracket
			if (tabs > 0) s << tabString;
			s << "{\n";

			// Expand the array entries
			switch (mType & 0xF)
			{
				// The following entries pack up to 16 values per line
				SAVE_MULTIPLE_CASE(int32, Int, Int);
				SAVE_MULTIPLE_CASE(uint32, UInt, UInt);
				SAVE_MULTIPLE_CASE(ushort, UShort, UShort);

				// Normal single value per line entries
				TO_STRING_ARRAY_CASE(bool, Bool, Bool);
				TO_STRING_ARRAY_CASE(Vector2i, Vector2i, Short2);
				TO_STRING_ARRAY_CASE(float, Float, Float);
				TO_STRING_ARRAY_CASE(Vector2f, Vector2f, Float2);
				TO_STRING_ARRAY_CASE(Vector3f, Vector3f, Float3);
				TO_STRING_ARRAY_CASE(Quaternion, Quaternion, Float4);
				TO_STRING_ARRAY_CASE(Color4ub, Color4ub, Color);

				case Type::String:
				{
					const Array<String>& a = AsStringArray();

					for (uint i = 0; i < a.GetSize(); ++i)
					{
						if (tabs > 0) s << tabString;
						s << "\t\"";
						s << a[i];
						s << "\"\n";
					}
				}
				break;
			}

			// Expand the closing bracket
			if (tabs > 0) s << tabString;
			s << "}";
			return true;
		}
		else
		{
			switch (mType)
			{
				case Type::Bool:	s << AsBool();			return true;
				case Type::Int:		s << AsInt();			return true;
				case Type::UInt:	s << AsUInt();			return true;
				case Type::UShort:	s << AsUShort();		return true;
				case Type::Short2:	s << AsVector2i();		return true;
				case Type::Float:	s << AsFloat();			return true;
				case Type::Float2:	s << AsVector2f();		return true;
				case Type::Float3:	s << AsVector3f();		return true;
				case Type::Float4:	s << AsQuaternion();	return true;
				case Type::Color:	s << AsColor4ub();		return true;

				case Type::String:
				{
					s << "\"";
					s << AsString();
					s << "\"";
				}
				return true;
			}
		}
	}
	return true;
}

//============================================================================================================
// Serialization from string format
//============================================================================================================

bool Variable::SerializeFrom (const String& s)
{
	Release();
	uint start = 0;
	uint end = s.GetLength();
	bool isArray = false;
	byte type = Type::Invalid;

	// Try to find out if this entry is an array
	if (end > 3 && s[end - 1] == '}')
	{
		// Skip the closing bracket
		--end;

		// Run through the first few characters
		for (uint i = start, imax = end; i < imax; ++i)
		{
			// Invalid (end of line) character found
			if (s[i] < ' ') break;
			
			// If we find empty array brackets, then this is an array
			if (s[i] == '[' && s[i+1] == ']')
			{
				// If we have some data before the array indicator, try to determine the type
				if (i > 0)
				{
					String strType;
					s.GetWord(strType, 0, i);

					if		(strType == "Bool")		type = Type::Bool;
					else if (strType == "Int")		type = Type::Int;
					else if (strType == "UInt")		type = Type::UInt;
					else if (strType == "UShort")	type = Type::UShort;
					else if (strType == "Short2")	type = Type::Short2;
					else if (strType == "Float")	type = Type::Float;
					else if (strType == "Float2")	type = Type::Float2;
					else if (strType == "Float3")	type = Type::Float3;
					else if (strType == "Float4")	type = Type::Float4;
					else if (strType == "Color")	type = Type::Color;
				}

				// Skip the array indicator
				start = i + 2;
				
				// Skip the characters leading up to the opening bracket
				for (;;)
				{
					if (start < end)
					{
						char ch = s[start];

						if (ch < 33)
						{
							++start;
							continue;
						}
						else if (ch == '{')
						{
							// We are now officially working with an array
							isArray = true;
							++start;
						}
					}
					break;
				}
				// Either way we're done after finding the square brackets
				break;
			}
		}
	}

	// If we're working with an array and there is data to work with
	if (isArray && (start < end))
	{
		String line, word;
		uint length;

		while (start < end)
		{
			// Retrieve lines one at a time
			start = s.GetLine(line, start, end);
			length = line.GetLength();
			
			if (length > 0)
			{
				// If we haven't determined the type of the array, let's do that now
				if (type == Type::Invalid)
				{
					type = Parse(line, 0);
					if (type == Type::Invalid) return false;
				}

				switch (type & 0xF)
				{
					// This macro gives these types the ability to parse more than one entry per line
					PARSE_MULTIPLE_CASE(int32, Int, Int);
					PARSE_MULTIPLE_CASE(uint32, UInt, UInt);
					PARSE_MULTIPLE_CASE(ushort, UShort, UShort);

					// Regular single entry per line parsing is more straightforward
					case Type::Bool:	line >> ToBoolArray().Expand();			break;
					case Type::Short2:	line >> ToVector2iArray().Expand();		break;
					case Type::Float:	line >> ToFloatArray().Expand();		break;
					case Type::Float2:	line >> ToVector2fArray().Expand();		break;
					case Type::Float3:	line >> ToVector3fArray().Expand();		break;
					case Type::Float4:	line >> ToQuaternionArray().Expand();	break;
					case Type::Color:	line >> ToColor4ubArray().Expand();		break;
					case Type::String:
					{
						if (length > 1 && line[0] == '"' && line[length - 1] == '"')
						{
							// If the string is wrapped in quotes, get rid of them
							line.GetString(ToStringArray().Expand(), 1, length - 1);
						}
						else
						{
							// Plain string -- just append it to the end
							line >> ToStringArray().Expand();
						}
						break;
					}
				}
			}
		}
		return (mType != Type::Invalid);
	}
	return (Parse(s, this) != Type::Invalid);
}

//============================================================================================================
// Conversion to boolean
//============================================================================================================

bool Variable::operator >> (bool& value) const
{
	if (IsBool())
	{
		value = AsBool();
		return true;
	}
	else if (IsInt())
	{
		value = (AsInt() != 0);
		return true;
	}
	else if (IsUInt())
	{
		value = (AsUInt() != 0);
		return true;
	}
	else if (IsUShort())
	{
		value = (AsUShort() != 0);
		return true;
	}
	else if (IsFloat())
	{
		value = Float::IsNotZero(AsFloat());
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to integer
//============================================================================================================

bool Variable::operator >> (int& value) const
{
	if (IsInt())
	{
		value = AsInt();
		return true;
	}
	else if (IsBool())
	{
		value = AsBool() ? 1 : 0;
		return true;
	}
	else if (IsUInt())
	{
		value = (int)AsUInt();
		return true;
	}
	else if (IsUShort())
	{
		value = (int)AsUShort();
		return true;
	}
	else if (IsFloat())
	{
		value = Float::RoundToInt(AsFloat());
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to unsigned integer
//============================================================================================================

bool Variable::operator >> (uint& value) const
{
	if (IsUInt())
	{
		value = AsUInt();
		return true;
	}
	else if (IsBool())
	{
		value = AsBool() ? 1 : 0;
		return true;
	}
	else if (IsInt())
	{
		value = (uint)AsInt();
		return true;
	}
	else if (IsUShort())
	{
		value = AsUShort();
		return true;
	}
	else if (IsFloat())
	{
		value = (uint)Float::RoundToInt(AsFloat());
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to unsigned short
//============================================================================================================

bool Variable::operator >> (ushort& value) const
{
	if (IsUShort())
	{
		value = AsUShort();
		return true;
	}
	else if (IsBool())
	{
		value = AsBool() ? 1 : 0;
		return true;
	}
	else if (IsInt())
	{
		value = (ushort)AsInt();
		return true;
	}
	else if (IsUInt())
	{
		value = (ushort)AsUInt();
		return true;
	}
	else if (IsFloat())
	{
		value = (ushort)Float::RoundToInt(AsFloat());
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to float
//============================================================================================================

bool Variable::operator >> (float& value) const
{
	if (IsFloat())
	{
		value = AsFloat();
		return true;
	}
	else if (IsBool())
	{
		value = AsBool() ? 1.0f : 0.0f;
		return true;
	}
	else if (IsInt())
	{
		value = (float)AsInt();
		return true;
	}
	else if (IsUInt())
	{
		value = (float)AsUInt();
		return true;
	}
	else if (IsUShort())
	{
		value = (float)AsUShort();
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Vector2i
//============================================================================================================

bool Variable::operator >> (Vector2i& value) const
{
	if (IsVector2i())
	{
		value = AsVector2i();
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Vector2f
//============================================================================================================

bool Variable::operator >> (Vector2f& value) const
{
	if (IsVector2f())
	{
		value = AsVector2f();
		return true;
	}
	else if (IsVector2i())
	{
		const Vector2i& v = AsVector2i();
		value.Set(v.x, v.y);
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Vector3f
//============================================================================================================

bool Variable::operator >> (Vector3f& value) const
{
	if (IsVector3f())
	{
		value = AsVector3f();
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Quaternion
//============================================================================================================

bool Variable::operator >> (Quaternion& value) const
{
	if (IsQuaternion())
	{
		value = AsQuaternion();
		return true;
	}
	else if (IsVector3f())
	{
		value.SetFromDirection( AsVector3f() );
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Color3f
//============================================================================================================

bool Variable::operator >> (Color3f& value) const
{
	if (IsColor3f())
	{
		value = AsColor3f();
		return true;
	}
	else if (IsColor4f())
	{
		// This operation is safe because Color4f is just like Color3f but with an extra float at the end
		value = AsColor3f();
		return true;
	}
	else if (IsColor4ub())
	{
		const Color4ub& c = AsColor4ub();
		value.Set( Float::FromRangeByte(c.r), Float::FromRangeByte(c.g), Float::FromRangeByte(c.b) );
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Color4f
//============================================================================================================

bool Variable::operator >> (Color4f& value) const
{
	if (IsColor4f())
	{
		value = AsColor4f();
		return true;
	}
	else if (IsColor3f())
	{
		value = AsColor3f();
		return true;
	}
	else if (IsColor4ub())
	{
		value = AsColor4ub();
		return true;
	}
	return false;
}

//============================================================================================================
// Conversion to Color4ub
//============================================================================================================

bool Variable::operator >> (Color4ub& value) const
{
	if (IsColor4ub())
	{
		value = AsColor4ub();
		return true;
	}
	else if (IsColor3f())
	{
		value = AsColor3f();
		return true;
	}
	else if (IsColor4f())
	{
		value = AsColor4f();
		return true;
	}
	return false;
}