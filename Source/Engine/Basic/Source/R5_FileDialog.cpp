#include "../Include/_All.h"
using namespace R5;

#ifdef _WINDOWS
#include <windows.h>
#endif

//============================================================================================================
// Clear the previously set properties
//============================================================================================================

void FileDialog::Clear()
{
	mFilename.Clear();
	mNames.Clear();
	mExtensions.Clear();
}

//============================================================================================================
// Adds a new filter, such as "R5 Ascii" with extension of "r5a"
//============================================================================================================

void FileDialog::AddFilter (const String& name, const String& extension)
{
	mNames.Expand() = name;
	mExtensions.Expand() = extension;
}

//============================================================================================================
// Windows
//============================================================================================================

#ifdef _WINDOWS

bool FileDialog::Show (const char* title, bool existingFilesOnly)
{
	OPENFILENAME ofn;
	char szFile[512] = {0};
	Array<char> filterData;
	String extension;

	// Clear the memory
	memset(&ofn, 0, sizeof(OPENFILENAME));

	// If we have a filename to work with...
	if (mFilename.IsValid())
	{
		// Use its filename as the default value and assume its extension
		strcpy(szFile, mFilename.GetBuffer());
		extension = System::GetExtensionFromFilename(mFilename);
	}
	else if (mExtensions.IsValid() && !mExtensions[0].Contains("*"))
	{
		// Assume a default extension
		extension = mExtensions[0].GetBuffer();
	}

	// Set up the filter string
	FOREACH(i, mNames)
	{
		const String& name	 = mNames[i];
		const String& filter = mExtensions[i];

		// Set the starting extension index (1-based)
		if (filter == extension) ofn.nFilterIndex = i+1;

		FOREACH(b, name) filterData.Expand() = name[b];
		filterData.Expand() = 0;
		filterData.Expand() = '*';
		filterData.Expand() = '.';

		FOREACH(b, filter) filterData.Expand() = filter[b];
		filterData.Expand() = 0;
	}
	filterData.Expand() = 0;

	// Set up the dialog properties
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.lpstrFile		= szFile;
	ofn.nMaxFile		= sizeof(szFile);
	ofn.lpstrFilter		= filterData.IsValid() ? filterData.GetBuffer() : "All\0*.*\0";
	ofn.lpstrTitle		= title;
	ofn.Flags			= OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt		= extension.GetBuffer();

	if (existingFilesOnly)
	{
		// Show the open file dialog
		ofn.Flags |= OFN_FILEMUSTEXIST;
		if (!GetOpenFileName(&ofn)) return false;
	}
	else
	{
		// Show the save file dialog
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		if (!GetSaveFileName(&ofn)) return false;
	}
	mFilename = szFile;
	return true;
}

#else

//============================================================================================================
// Other operating systems
//============================================================================================================

bool FileDialog::Show (const char* title, bool existingFilesOnly)
{
	ASSERT(false, "Functionality not yet implemented");
	return false;
}

#endif