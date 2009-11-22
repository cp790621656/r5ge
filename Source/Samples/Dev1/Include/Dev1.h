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

	bool OnChangeDelegate	 (Area* area);
	bool OnBrightnessChange	 (Area* area);
	bool OnRangeChange		 (Area* area);
	bool OnPowerChange		 (Area* area);
	bool OnButtonStateChange (Area* area);
};
}