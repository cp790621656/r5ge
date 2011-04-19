#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Noise testing application
// Author: Michael Lyashenko
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