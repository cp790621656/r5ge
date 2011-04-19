#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Development Testing Application
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

	OSDrawDeferred*	mDraw;

public:

	TestApp();
	~TestApp();

	void Run();
	uint OnKeyPress (const Vector2i& pos, byte key, bool isDown);
};
}