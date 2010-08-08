#include "../Include/_All.h"
using namespace R5;

#ifdef _WINDOWS
  #include <windows.h>
#elif defined _MACOS
  #import <Foundation/NSString.h>
  #import <Foundation/NSAutoreleasePool.h>
  #import <AppKit/NSOpenPanel.h>
  #import <AppKit/NSApplication.h>
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
	// Windows file dialog changes the current working directory... sigh.
	String currentPath (System::GetCurrentPath());

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

	bool retVal;

	if (existingFilesOnly)
	{
		// Show the open file dialog
		ofn.Flags |= OFN_FILEMUSTEXIST;
		retVal = GetOpenFileName(&ofn) != 0;
	}
	else
	{
		// Show the save file dialog
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		retVal = GetSaveFileName(&ofn) != 0;
	}

	if (retVal)
	{
		// Remember the filename and restore the current working directory
		mFilename = szFile;
		System::SetCurrentPath(currentPath);
	}
	return retVal;
}

#elif defined _MACOS

//============================================================================================================
// Mac OSX
//============================================================================================================

bool FileDialog::Show (const char* title, bool existingFilesOnly)
{
	// Managed Obj-C should be wrapped into an auto-release pool so it doesn't leak memory
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];

	// Create the File Open Dialog class.
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];

	// Enable the selection of files in the dialog.
	[openDlg setCanChooseFiles:YES];

	// Disable the selection of directories in the dialog.
	[openDlg setCanChooseDirectories:NO];

	// Reuse the same filename
	NSString* nsFile = [NSString stringWithUTF8String:mFilename.GetBuffer()];

	// Display the dialog
	if ( [openDlg runModalForDirectory:nil file:nsFile] == NSOKButton )
	{
		nsFile = [openDlg filename];
		mFilename = [nsFile UTF8String];
		[pool drain];
		return true;
	}
	[pool drain];
	return false;
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