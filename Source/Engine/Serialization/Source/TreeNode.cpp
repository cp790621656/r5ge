#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves a single phrase from the string
//============================================================================================================

bool GetPhrase (const String& s, uint& from, uint to, String& out)
{
	if (from < to)
	{
		// Skip all spaces and tabs
		while (from < to && s[from] < 33) ++from;

		char ch = s[from];

		// If we haven't reached the end and have a valid character to work with
		if ( (from != to) && (ch < '{') && (ch != '=') )
		{
			uint phraseEnd = from + 1;
			uint lastChar = from;

			// Find where this phrase ends
			while (phraseEnd < to)
			{
				ch = s[phraseEnd];

				// If it's an empty character, keep going
				if (ch == ' ' || ch == '\t')
				{
					++phraseEnd;
				}
				else
				{
					// If we found a comment, the rest of the line should be ignored
					if (ch == '/' && (phraseEnd + 1 < to) && s[phraseEnd+1] == '/')
					{
						for (phraseEnd += 2; phraseEnd < to; ++phraseEnd)
						{
							if (s[phraseEnd] < ' ') break;
						}
						break;
					}

					// If it's an end-of-phrase character, stop
					if (ch < ' ' || ch == '=' || ch > 'z' || ch == '}') break;

					// Remember the last valid character
					lastChar = phraseEnd;
					++phraseEnd;
				}
			}

			// Get the substring and update the starting position to the last processed character
			if ( s.GetString(out, from, lastChar + 1) )
			{
				from = phraseEnd;

				// Skip trailing spaces
				while (from < to && s[from] < 33) ++from;
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================
// Retrieves a segment of bracketed text. 'out' will be set to the text inside the brackets, and
// 'from' will be updated to be past the closing bracket.
//============================================================================================================
// Example: "abc[] { 123 } More = { 321 }"
// Feeding the above string will return "abc[] { 123 }" in the 'out' parameter.
//============================================================================================================

bool GetSegment (const String& s, uint& from, uint to, String& out)
{
	if (from < to)
	{
		// Skip empty characters
		while (from < to && s[from] < 33) ++from;

		out.Clear();
		int brackets = 0;
		bool inQuotes = false;

		for (uint i = from; i < to; )
		{
			char ch = s[i];

			if (ch == '"')
			{
				inQuotes = !inQuotes;
			}
			else if (inQuotes)
			{
				// Append this character
				out.Expand() = ch;
			}
			else if (ch == '/' && (i + 1 < to) && s[i+1] == '/')
			{
				// If a C++ style comment is encountered...
				// Skip the rest of the line
				for (i += 2; i < to; ++i)
				{
					if (s[i] < ' ') break;
				}
				continue;
			}
			else
			{
				// Append this character
				out.Expand() = ch;

				if (ch == '{')
				{
					++brackets;
				}
				else if (ch == '}')
				{
					if (--brackets < 1)
					{
						// Move up the starting position
						from = i + 1;
						
						// Skip empty characters
						while (from < to && s[from] < 33) ++from;
						return true;
					}
				}
			}
			++i;
		}
	}
	return false;
}

//============================================================================================================
// Saves to the specified file, using the file's extension to determine whether it should be binary
//============================================================================================================

bool TreeNode::Save (const char* filename) const
{
	String s(filename);
	bool ascii = s.EndsWith(".r5a") || s.EndsWith(".txt");
	return Save(filename, !ascii);
}

//============================================================================================================
// Serialization to a file
//============================================================================================================

bool TreeNode::Save (const char* filename, bool binary) const
{
	if (binary)
	{
		Memory m;
		m.Append("//R5B", 5);
		return SerializeTo(m) && m.Save(filename);
	}
	else
	{
		String s;
		s << "//R5A\n\n";
		return SerializeTo(s) && s.Save(filename);
	}
}

//============================================================================================================
// Loads a file from disk
//============================================================================================================

bool TreeNode::Load (const char* filename)
{
	Memory mem;
	return mem.Load(filename) ? Load(mem.GetBuffer(), mem.GetSize()) : false;
}

//============================================================================================================
// Loads the tree from memory loaded elsewhere
//============================================================================================================

bool TreeNode::Load (const byte* buffer, uint size)
{
	// R5 format files always begin with '//R5', followed by 'A' for ASCII or 'B' for Binary
	if (size > 5 &&
		buffer[0] == '/' &&
		buffer[1] == '/' &&
		buffer[2] == 'R' &&
		buffer[3] == '5')
	{
		char type = buffer[4];

		size   -= 5;
		buffer += 5;

		if (type == 'A')
		{
			// Copy the memory into a string
			String s;
			memcpy(s.Resize(size), buffer, size);
			s[size] = 0;

			// Load the tree
			return SerializeFrom(s);
		}
		else if (type == 'B')
		{
			// Serialize directly from memory
			return SerializeFrom(buffer, size);
		}
	}
	return false;
}

//============================================================================================================
// Serialization to a memory buffer
//============================================================================================================

bool TreeNode::SerializeTo (Memory& mem) const
{
	if (IsValid())
	{
		// Write down the tag
		mem.Append(mTag);

		// Save the variable
		mValue.SerializeTo(mem);

		// Write down the number of children
		uint children = mChildren.GetSize();
		mem.AppendSize(children);

		// Go through each child and serialize them one at a time
		for (uint i = 0; i < children; ++i) mChildren[i].SerializeTo(mem);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization to a string
//============================================================================================================

inline void WriteTabs (String& s, uint count) { for (uint i = 0; i < count; ++i)  s << "\t"; }

bool TreeNode::SerializeTo (String& s, uint level) const
{
	if (IsValid())
	{
		// Write down the node's tag
		WriteTabs(s, level);
		s << mTag;

		// If the node has a value, write it down as well
		if (mValue.IsValid())
		{
			s << " = ";
			mValue.SerializeTo(s, level);
		}

		// Continue onto the next line
		s << "\n";

		// If the node has children, we should save them as well
		if (HasChildren())
		{
			// Opening bracket
			WriteTabs(s, level);
			s << "{\n";

			// Lines with children should be separated by extra lines for clarity purposes
			bool extraLine = false;

			// Run through all children and save them one at a time
			for (uint i = 0; i < mChildren.GetSize(); ++i)
			{
				const TreeNode& child = mChildren[i];
				
				if (child.IsValid())
				{
					bool hasChildren = child.HasChildren() || child.mValue.IsArray();

					if (extraLine || hasChildren)
					{
						if (i > 0) s << "\n";
						extraLine = hasChildren;
					}

					// Serialize the child
					child.SerializeTo(s, level + 1);
				}
			}

			// Closing bracket
			WriteTabs(s, level);
			s << "}\n";
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Recursive binary loading function
//============================================================================================================

bool TreeNode::SerializeFrom (ConstBytePtr& buffer, uint& size)
{
	Release();

	if (Memory::Extract(buffer, size, mTag) && mValue.SerializeFrom(buffer, size))
	{
		uint children;
		
		if (Memory::ExtractSize(buffer, size, children))
		{
			for (uint i = 0; i < children; ++i)
			{
				TreeNode& child = mChildren.Expand();
				
				if (!child.SerializeFrom(buffer, size))
				{
					mChildren.Shrink();
					return false;
				}
			}
			return true;
		}
	}
	ASSERT(false, "Binary format parsing error!");
	return false;
}

//============================================================================================================
// Serialize from the string format. Note that the text must be already clean -- no comments!
//============================================================================================================

bool TreeNode::SerializeFrom (const String& s)
{
	Release();
	uint from = 0, to = s.GetLength();

	// Get the node's tag
	if (GetPhrase(s, from, to, mTag))
	{
		// Serialize from this string
		return ParseProperties(s, from, to);
	}
	return false;
}

//============================================================================================================
// Recursive text format parsing
//============================================================================================================

bool TreeNode::ParseProperties (const String& s, uint& from, uint to)
{
	// If there is more, keep going
	if (from < to)
	{
		String temp;

		// If the from character is an equality sign, we must be expecting a temp
		if (s[from] == '=')
		{
			uint wordStart = ++from;

			if (s.GetWord(temp, wordStart, to))
			{
				// If the from word ends with square brackets, then the array must follow
				if (temp.EndsWith("[]"))
				{
					if (GetSegment(s, from, to, temp))
					{
						if (!mValue.SerializeFrom(temp))
						{
							ASSERT(false, "Failed to serialize the value");
							return false;
						}
					}
				}
				else if (GetPhrase(s, from, to, temp))
				{
					if (!mValue.SerializeFrom(temp))
					{
						ASSERT(false, "Failed to serialize the value");
						return false;
					}
				}
			}
		}

		// If the next character is an opening bracket, then this node has children
		if (from < to && s[from] == '{')
		{
			uint closingBracket = from++, counter = 1;

			// Find the correct closing bracket
			while (counter)
			{
				if (++closingBracket < to)
				{
					if		(s[closingBracket] == '}')  --counter;
					else if (s[closingBracket] == '{')  ++counter;
				}
				else
				{
					ASSERT(false, "Unable to find the matching bracket");
					return false;
				}
			}

			// Keep going until the closing bracket is reached
			while (from < closingBracket)
			{
				// Get the node's tag, and if failed just break out
				if (!GetPhrase(s, from, to, temp)) break;

				// Add this child
				TreeNode& child = mChildren.Expand();
				child.mTag = temp;

				// Serialize the child -- if failed, just break out
				if (!child.ParseProperties(s, from, closingBracket))
				{
					mChildren.Shrink();
					break;
				}
			}

			// Skip the closing bracket
			from = closingBracket + 1;
		}
		return true;
	}
	return true;
}