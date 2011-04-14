#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Dev6 - Skeletal Animation
// Author: Michael Lyashenko
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
	void OnTechnique	(UIWidget* widget, uint state, bool isSet);
	void PlayAnimation	(Model* model, const String& name, float speed = 1.0f);
};
}