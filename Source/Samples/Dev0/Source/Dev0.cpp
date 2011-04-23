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
	in.Load("../../../Resources/Shaders/Surface/Diffuse + Normal.shader");

	printf("Original: %u bytes\n", in.GetSize());

	GLUnifiedShader shader;
	shader.SerializeFrom(in);
	
	String out;

	Flags flags;
	flags.Set(IShader::Flag::Vertex, true);
	shader.SerializeTo(out, flags);
	printf("================= VERTEX\n%s=================\n", out.GetBuffer());

	flags.Clear();
	flags.Set(IShader::Flag::Fragment, true);
	shader.SerializeTo(out, flags);
	printf("================= FRAGMENT\n%s=================\n", out.GetBuffer());

	getchar();
	return 0;
}
