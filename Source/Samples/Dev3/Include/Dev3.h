#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Development Testing Application
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;
	Scene		mScene;
	Camera*		mCam;
	bool		mFlag;
	String		mDebug;
	uint		mObjects;

	IGraphics::FrameStats	mStats;
	Scene::Techniques		mForward;

public:

	TestApp();
	~TestApp();

	void  Init();
	void  Run();
	void  OnDraw();
	float UpdateStats();
};
}