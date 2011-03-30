#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// File dialog window (implemented natively on each system)
//============================================================================================================

class FileDialog
{
protected:

	String mFilename;
	Array<String> mNames;
	Array<String> mExtensions;

public:

	// Gets the previously set filename
	const String& GetFilename() const { return mFilename;  }

	// Sets the filename manually
	void SetFilename (const String& file) { mFilename = file; }

	// Clear the previously set properties
	void Clear();

	// Adds a new filter, such as "R5 Ascii" with extension of "r5a"
	void AddFilter (const String& name, const String& extension);

	// Show the file dialog and update the 'mFilename'
	bool Show (const char* title, bool existingFilesOnly = true);
};