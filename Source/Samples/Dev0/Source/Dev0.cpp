//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

#ifdef _WINDOWS
#include <windows.h>
#else
#include <dirent.h>
#endif

//============================================================================================================
// GNU version
//============================================================================================================

bool ReadFolder (const String& dir, Array<String>& folders, Array<String>& files)
{
	String file;

#ifdef _WINDOWS
	WIN32_FIND_DATA info;
	void* fileHandle = FindFirstFile((dir + "/*.*").GetBuffer(), &info);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do 
		{
			file = info.cFileName;

			if (!file.BeginsWith("."))
			{
				if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					folders.Expand() = file;
				}
				else
				{
					files.Expand() = file;
				}
			}
		}
		while (FindNextFile(fileHandle, &info));
	}

#else
	DIR* dp = opendir(dir.GetBuffer());
	if (dp == 0) return false;
	struct dirent* dirp = 0;

	while ((dirp = readdir(dp)) != 0)
	{
		if (dirp->d_type == DT_DIR)
		{
			folders.Expand() = dirp->d_name;
			if (folders.Back().BeginsWith(".")) folders.Shrink();
		}
		else
		{
			files.Expand() = dirp->d_name;
		}
	}
    closedir(dp);
#endif
    return true;
}

//============================================================================================================
// Application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");

	Array<String> folders;
	Array<String> files;

	ReadFolder(".", folders, files);

	folders.Sort();
	files.Sort();

	FOREACH(i, folders)
	{
		printf("[%s]\n", folders[i].GetBuffer());
	}

	FOREACH(i, files)
	{
		printf("%s\n", files[i].GetBuffer());
	}

#ifdef _WINDOWS
	OPENFILENAME ofn;
	char szFile[260];
	szFile[0] = 0;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = 0;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		printf("File: %s\n", ofn.lpstrFile);
	}
#endif
	printf("Done!\n");
	getchar();
	return 0;
}