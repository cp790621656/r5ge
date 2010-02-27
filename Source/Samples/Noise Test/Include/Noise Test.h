#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Noise testing application
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Noise/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	Core*		mCore;
	Scene		mScene;
	Camera*		mCam;
	UI*			mUI;
	Noise		mNoise;
	bool		mRegenerate;

public:

	TestApp();
	~TestApp();

	void InitUI();
	void Run();
	void OnDraw();
	uint DrawScene();
	void Regenerate();

	void Generate (UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	void UpdateTooltips (UIWidget* widget, uint state, bool isSet);
};
};