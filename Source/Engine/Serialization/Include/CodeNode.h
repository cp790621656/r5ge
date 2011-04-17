#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Helper class designed to make it easier to parse C-based syntax files.
// Author: Michael Lyashenko
//============================================================================================================

struct CodeNode
{
	String mLine;
	Array<CodeNode> mChildren;

	// Load the contents of the specified string into memory, stripping out C++ comments and extra spaces
	void Load (const String& source);

	// Load the specified segment
	uint Load (const String& source, uint start, uint end);

	// Save the structure's contents out into the specified string
	void Save (String& out, uint tabs = 0);
};