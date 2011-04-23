#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Helper class designed to make it easier to parse C-based syntax files.
// Author: Michael Lyashenko
//============================================================================================================

struct CodeNode
{
	String mLine;
	Array<CodeNode> mChildren;

	// Release the code stored in the class
	void Release() { mLine.Clear(); mChildren.Release(); }

	// Load the contents of the specified string into memory, stripping out C++ comments and extra spaces
	void SerializeFrom (const String& source);

	// Load the specified segment
	uint SerializeFrom (const String& source, uint start, uint end);

	// Save the structure's contents out into the specified string
	void SerializeTo (String& out, uint tabs = 0) const;

	// Save the structure's contents out into the specified string, using the specified list of #defines
	void SerializeTo (String& out, const Array<String>& defines) const;

	// Save the structure's contents into the specified node using the specified list of #defines
	void SerializeTo (CodeNode& out, const Array<String>& defines) const;
};