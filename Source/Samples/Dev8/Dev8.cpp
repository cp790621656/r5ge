//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
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
		rot.Rotate(axis, Time::GetDelta() * 0.2f);
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

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

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
		while (mCore->Update());
		//*mCore >> "Config/Dev8.txt";
	}
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