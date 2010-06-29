//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

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

	System::ReadFolder(".", folders, files);

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

	FileDialog dlg;
	dlg.AddFilter("R5 Ascii", "r5a");
	dlg.AddFilter("R5 Binary", "r5b");
	dlg.AddFilter("R5 Compressed", "r5c");
	dlg.AddFilter("Test", "r5a|*.r5b");
	dlg.AddFilter("All", "*");

	if (dlg.Show("Open", true))
	{
		printf("Result: [%s]\n", dlg.GetFilename().GetBuffer());
	}

	if (dlg.Show("Save as...", false))
	{
		printf("Result: [%s]\n", dlg.GetFilename().GetBuffer());
	}

	printf("Done!\n");
	getchar();
	return 0;
}