#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Test Application
//============================================================================================================

class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;
	Camera*		mCam;
	Scene		mScene;

public:

	TestApp();
	~TestApp();

	void Run();
	void OnDraw();
};