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
	String in;
	in.Load("../../../Resources/Shaders/Surface/Skinned.shader");

	printf("Original: %u bytes\n", in.GetSize());

	CodeNode code;
	code.Load(in);
	
	String out;

	FOREACH(i, code.mChildren)
	{
		CodeNode& c = code.mChildren[i];

		if (c.mLine.Replace("void Vertex()", "void main()", true))
		{
			c.Save(out);
			printf("==========VERTEX==============\n");
			printf("%s\n", out.GetBuffer());
			printf("==============================\n");
			out.Clear();
		}
		else if (c.mLine.Replace("void Fragment()", "void main()", true))
		{
			c.Save(out);
			printf("==========FRAGMENT============\n");
			printf("%s\n", out.GetBuffer());
			printf("==============================\n");
			out.Clear();
		}
		else c.Save(out);
	}
	getchar();
	return 0;
}
