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

public:

	TestApp();
	~TestApp();

	void Run();
	void OnChangeDelegate	 (UIWidget* widget);
	void OnBrightnessChange	 (UIWidget* widget);
	void OnRangeChange		 (UIWidget* widget);
	void OnPowerChange		 (UIWidget* widget);
	void OnButtonStateChange (UIWidget* widget, uint state, bool isSet);
};
}