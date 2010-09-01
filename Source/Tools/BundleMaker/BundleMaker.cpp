//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// BundleMaker is a tool that can create and extract asset bundles
//============================================================================================================

#include "../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================

#define LOAD_DONE	printf("Loaded %s\n", filename.GetBuffer())
#define SAVE_DONE	printf("Saved out '%s'\n", filename.GetBuffer())
#define READ_ERROR	printf("ERROR: Unable to read '%s'\n", filename.GetBuffer())
#define WRITE_ERROR printf("ERROR: Unable to write '%s'\n", filename.GetBuffer())

//============================================================================================================
// Main application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif

	uint errors = 0;

	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
		{
			String filename (argv[i]);

			// TODO: If the file ends with 'r5d', extract all assets within.
			// If not, go through all specified files/folders and add them to the bundle, then save it.
		}
	}

	if (errors > 0)
	{
		puts("Press Enter to exit...");
		getchar();
	}
	return 0;
}