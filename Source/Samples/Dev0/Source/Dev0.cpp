//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
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
	System::SetCurrentPath("../../../Resources");

	String shader;

	if (shader.Load("Shaders/Surface/Simple.shader"))
	{
		CodeNode root;
		root.SerializeFrom(shader);

		Array<String> defines;
		defines.Expand() = "Fragment";
		defines.Expand() = "Lit";
		//defines.Expand() = "Deferred";

		shader.Clear();
		root.SerializeTo(shader, defines);

		puts("Shader:");
		puts(shader.GetBuffer());
	}
	else puts("Not found");
#ifdef _WINDOWS
	getchar();
#endif
	return 0;
}
