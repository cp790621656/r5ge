//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// ConfigConverter can be used to convert R5A and R5B files into R5C and vice versa
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
		TreeNode root;

		for (int i = 1; i < argc; ++i)
		{
			String filename (argv[i]);

			if (root.Load(filename))
			{
				LOAD_DONE;

				if (filename.EndsWith(".r5a"))
				{
					filename.Replace(".r5a", ".r5c");
				}
				else
				{
					String ext (System::GetExtensionFromFilename(filename));

					if (ext.IsValid())
					{
						filename.Replace(ext, "r5a", true);
					}
					else
					{
						filename << ".r5a";
					}
				}

				if (root.Save(filename.GetBuffer()))
				{
					SAVE_DONE;
				}
				else
				{
					WRITE_ERROR;
					++errors;
				}
			}
			else
			{
				READ_ERROR;
				++errors;
			}
		}
	}
	else
	{
		++errors;
		puts("R5 Config Converter Tool v.1.0.0 by Michael Lyashenko");
		puts("Usage: ConfigConverter file0 [file1] [file2] [...]");
		puts("Example 1: FontConverter HelloWorld.r5a");
		puts("Example 2: FontConverter HelloWorld.r5b HelloWorld.r5c");
		puts("You can also drag the file in question onto this executable in order to convert it.");
	}

	if (errors > 0)
	{
		puts("Press Enter to exit...");
		getchar();
	}
	return 0;
}