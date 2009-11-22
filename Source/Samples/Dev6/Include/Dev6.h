#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
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
	ModelInstance*	mInst0;
	ModelInstance*	mInst1;
	Camera*			mCam;
	Label*			mStatus;
	Label*			mMode;
	ITechnique*		mTech;

public:

	TestApp();
	~TestApp();

	void Run();
	void OnDraw();

	Model* GetModel0()		{ return (mInst0 != 0 ? mInst0->GetModel() : 0); }
	Model* GetModel1()		{ return (mInst1 != 0 ? mInst1->GetModel() : 0); }
	Model* GetRandomModel() { return (Time::GetMilliseconds() % 2 == 1) ? GetModel0() : GetModel1(); }

	void ToggleTechnique();
	void PlayAnimation		(Model* model, const String& name, float speed = 1.0f);
	void StopAnimation		(Model* model);
	void OnAnimationStatus	(Model* model, const Animation* anim, float timeToEnd);

	bool OnKey (const Vector2i& pos, byte key, bool isDown);
};
}