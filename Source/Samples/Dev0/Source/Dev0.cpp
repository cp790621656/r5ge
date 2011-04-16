//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
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
	Memory in;
	in.Load("../../../Resources/Textures/Stone/rocky_ns.png");

	printf("Original: %u bytes\n", in.GetSize());

	Image img;
	
	if (img.Load(in.GetBuffer(), in.GetSize()))
	{
		Memory out;

		if (img.Save(out, "R5T")) printf("R5T: %u bytes\n", out.GetSize());

		if (img.Load(out.GetBuffer(), out.GetSize()))
		{
			printf("Loaded the image again, saving as TGA\n");

			if (img.Save("test.tga"))
			{
				printf("Saved\n");
			}
		}
		else puts("Failed to re-load the image");
	}
	getchar();
	return 0;
}
