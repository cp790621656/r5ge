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
	TreeNode root;
	root.Load("C:/Projects/r5ge/Resources/Models/shadow test.r5a");

	Memory in, out;
	root.SerializeTo(in);

	Time::Update();
	printf("[%4u] Original: %u bytes\n", Time::GetMilliseconds(), in.GetSize());

	if (Compress(in.GetBuffer(), in.GetSize(), out))
	{
		in.Clear();
		Time::Update();
		printf("[%4u] Compressed: %u bytes\n", Time::GetMilliseconds(), out.GetSize());

		if (Decompress(out.GetBuffer(), out.GetSize(), in))
		{
			String text;
			const byte* buffer = in.GetBuffer();
			uint size = in.GetSize();
			Time::Update();
			printf("[%4u] Decompressed: %u bytes", Time::GetMilliseconds(), in.GetSize());
			in.Save("c:/temp/out.txt");
		}
		else
		{
			puts("Failed to decompress");
		}
	}
	else
	{
		puts("Failed to compress");
	}
	getchar();
	return 0;
}