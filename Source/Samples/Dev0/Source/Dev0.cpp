//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
#include "../../../Engine/Interface/Include/_All.h"
using namespace R5;

struct Test
{
	R5_DECLARE_BASE_CLASS("Test", Test);
};

struct Hmm : public Test
{
	R5_DECLARE_INHERITED_CLASS("Hmm", Hmm, Test, Test);
};

//============================================================================================================
// Application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
	System::SetCurrentPath("../../../Resources");

	printf("Name: %s\n", Hmm::ClassName().GetBuffer());
	getchar();
	return 0;
}
