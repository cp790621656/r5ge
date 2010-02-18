//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Image/Include/_All.h"
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

	bool error = false;

	if (argc > 1)
	{
		Image img;

		for (int i = 1; i < argc; ++i)
		{
			if (img.Load(argv[i]))
			{
				String name (argv[i]);
				String extension (System::GetExtensionFromFilename(name));

				if (extension == "r5t")
				{
					name.Replace(extension, "tga");
				}
				else
				{
					name.Replace(extension, "r5t");
				}

				if (img.Save(name))
				{
					printf("Saved '%s'\n", name.GetBuffer());
				}
			}
			else
			{
				error = true;
				printf("Unable to load '%s'\n", argv[i]);
			}
		}
	}
	else
	{
		error = true;
		printf("Usage: Drag an image file you want to convert onto this executable\n");
	}

	if (error)
	{
		printf("Press any key to exit...\n");
		getchar();
	}
	return 0;
}