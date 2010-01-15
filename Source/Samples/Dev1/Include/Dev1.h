#pragma once

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
	Camera*		mCam;
	Scene		mScene;

public:

	TestApp();
	~TestApp();

	void Run();
	void OnDraw();
	bool OnKey (const Vector2i& pos, byte key, bool isDown);

	bool OnChangeDelegate	 (UIWidget* widget);
	bool OnBrightnessChange	 (UIWidget* widget);
	bool OnRangeChange		 (UIWidget* widget);
	bool OnPowerChange		 (UIWidget* widget);
	bool OnButtonStateChange (UIWidget* widget);
};
}