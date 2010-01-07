#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Dev6 - Skeletal Animation
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Camera*			mCam;
	UILabel*		mDebug[7];
	ITechnique*		mTech;
	Scene			mScene;

public:

	TestApp();
	~TestApp();

	void Run();
	void OnDraw();
	bool OnTechnique		(UIArea* area);
	void PlayAnimation		(Model* model, const String& name, float speed = 1.0f);
};
}