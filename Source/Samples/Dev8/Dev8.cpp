//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev8: Projected Textures
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================

class SlowTwirl : public Script
{
public:

	R5_DECLARE_INHERITED_CLASS("Slow Twirl", SlowTwirl, Script, Script);

	virtual void OnPreUpdate()
	{
		Quaternion rot = mObject->GetRelativeRotation();
		Vector3f axis (0.0f, 1.0f, 0.0f);
		rot.Rotate(axis, Time::GetDelta() * 0.0002f);
		mObject->SetRelativeRotation(rot);
	}
};

//============================================================================================================

class SlightRotation : public Script
{
private:

	Quaternion	mOriginal;

public:

	R5_DECLARE_INHERITED_CLASS("Slight Rotation", SlightRotation, Script, Script);

	virtual void OnInit() { mOriginal = mObject->GetRelativeRotation(); }

	virtual void OnPreUpdate()
	{
		Quaternion rot = mOriginal;
		float time = Time::GetTime();
		Vector3f dir ( Float::Cos(time) * 3.0f, 5.0f, Float::Sin(time) * 2.0f );
		mObject->SetRelativeRotation(dir * rot);
	}
};

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mScene;
	DebugCamera*	mCam;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp() : mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	Script::Register<SlightRotation>();
	Script::Register<SlowTwirl>();

	mCore->SetSleepDelay(0);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/Dev8.txt")
	{
		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Object::MouseMove, mCam) );
		mCore->SetListener( bind(&Object::Scroll, mCam) );

		while (mCore->Update()) {}

		//*mCore >> "Config/Dev8.txt";
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllDeferred(1);
}

//============================================================================================================

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");
	TestApp app;
    app.Run();
	return 0;
}