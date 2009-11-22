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

public:

	TestApp();
	~TestApp();

	void Run();
	void OnDraw();
	bool OnKey (const Vector2i& pos, byte key, bool isDown);

	bool OnChangeDelegate	 (UIArea* area);
	bool OnBrightnessChange	 (UIArea* area);
	bool OnRangeChange		 (UIArea* area);
	bool OnPowerChange		 (UIArea* area);
	bool OnButtonStateChange (UIArea* area);
};
}