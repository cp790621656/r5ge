#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

template <typename T> class Array;

namespace System
{
	struct OS
	{
		enum
		{
			Unknown			= 0x0,
			Windows			= 0x1,
			MacOS			= 0x2,
			Linux			= 0x4,
		};
	};

	uint	GetOS();											// Returns the current operating system
	int		Execute					(const char* command);		// Executes the specified command via shell
	bool	SetCurrentPath			(const char* path);			// _chdir()
	void	Log						(const char* format, ...);	// Dumps a string into the log file
	void	FlushLog();											// Flush the log file, saving out the contents
	bool	FileExists				(const char* filename);		// Checks if the file exists
	uint	GetFileHeader			(const char* filename);		// Gets the file's header (first 4 bytes)
	String	GetCurrentPath();									// Gets the current folder path (ends with '/')
	String	GetFilenameFromPath		(const String& path, bool extension = true); // "c:/temp/test.abc" becomes "test.abc"
	String	GetPathFromFilename		(const String& file);		// "c:/temp/test.abc" becomes "c:/temp/"
	String	GetExtensionFromFilename(const String& file);		// "c:/temp/test.abc" becomes "abc"
	String	GetNormalizedFilename	(const String& file);		// "c:/temp/../test.abc" becomes "c:/test.abc"

	// Reads the contents of the specified folder, populating file and folder lists
	bool ReadFolder (const String& dir, Array<String>& folders, Array<String>& files);

	// Fills out a list of all files with the partial path matching 'path'. Returns 'true' if one was found.
	bool GetFiles (const String& path, Array<String>& files, bool recursive = false);

	// Whether the filename is close enough to be a match
	// Flag 0 = Filename must be an exact match to 'name'
	// Flag 1 = Starts with 'name'
	// Flag 2 = Ends with 'name'
	bool IsFilenameCloseEnough (const String& filename,
		const String& dir, const String& name,
		const String& ext, byte flag = 0);

	// Returns the best matching filename that exists. Allows specifying a different extension than
	// that of the existing file. "c:/temp/test.abc" will match "c:/temp/test.txt" if it exists instead.
	String GetBestMatch (const String& filename);
};