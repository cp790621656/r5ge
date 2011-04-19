#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic R5 Application
// Author: Michael Lyashenko
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};
}