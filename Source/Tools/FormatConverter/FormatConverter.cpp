//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// ConfigConverter can be used to convert R5A and R5B files into R5C and vice versa
// Author: Michael Lyashenko
//============================================================================================================

#include "../../Engine/Image/Include/_All.h"
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

#ifndef _DEBUG
	if (argc > 1)
#endif
	{
		Memory mem;
		TreeNode root;
		Image img;
#ifndef _DEBUG
		for (int i = 1; i < argc; ++i)
#endif
		{
#ifndef _DEBUG
			String filename (argv[i]);
#else
			String filename ("c:/projects/r5ge/resources/models/shadow test 2.r5c");
#endif

			if (mem.Load(filename))
			{
				if (mem.GetSize() > 5)
				{
					LOAD_DONE;

					String ext (System::GetExtensionFromFilename(filename));

					const byte* buffer = mem.GetBuffer();
					byte id = buffer[4];
					bool isTreeNode = (id == 'A') || (id == 'B') || (id == 'C');

					if (!isTreeNode && img.Load(mem.GetBuffer(), mem.GetSize()))
					{
						filename.Replace(ext, ext == "r5t" ? "tga" : "r5t");

						if (img.Save(filename))
						{
							SAVE_DONE;
						}
						else
						{
							WRITE_ERROR;
							++errors;
						}
					}
					else if (root.Load(mem))
					{
						filename.Replace(ext, ext == "r5c" ? "r5a" : "r5c");

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
				READ_ERROR;
				++errors;
			}
		}
	}
#ifndef _DEBUG
	else
	{
		++errors;
		puts("R5 Format Converter Tool v.2.0.0 by Michael Lyashenko");
		puts("Usage: FormatConverter file0 [file1] [file2] [...]");
		puts("You can also drag the file in question onto this executable in order to convert it.");
	}
#endif

	if (errors > 0)
	{
		puts("Press Enter to exit...");
		getchar();
	}
	return 0;
}