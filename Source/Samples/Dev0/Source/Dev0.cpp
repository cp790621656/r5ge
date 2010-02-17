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

	Memory c, u;
	u.Load("Models/peasant.r5a");

	if (Compress(u, c))
	{
		printf("Compressed %u bytes down to %u bytes\n", u.GetSize(), c.GetSize());		
		u.Clear();

		if (Decompress(c, u))
		{
			printf("Decompressed %u bytes up to %u bytes\n", c.GetSize(), u.GetSize());
			u.Save("Models/peasant_out.r5a");
		}
		else
		{
			printf("Decompression failed!\n");
		}
	}
	else
	{
		printf("Compression failed!\n");
	}
	getchar();
	return 0;
}