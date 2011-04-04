//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev13: Shadows
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Render
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"

using namespace R5;

//============================================================================================================
// Light source control script
//============================================================================================================

class OSRotateLight : public Script
{
public:

	R5_DECLARE_INHERITED_CLASS("OSRotateLight", OSRotateLight, Script, Script);

	virtual void OnInit()	 { mObject->SubscribeToMouseMove(1000); }
	virtual void OnDestroy() { mObject->UnsubscribeFromMouseMove(1000); }

	virtual bool OnMouseMove (const Vector2i& pos, const Vector2i& delta)
	{
		if (mObject->GetCore()->IsKeyDown(Key::L))
		{
			Quaternion relativeRot (mObject->GetRelativeRotation());

			// Horizontal
			{
				Quaternion rotQuat ( Vector3f(0.0f, 0.0f, 1.0f), 0.25f * DEG2RAD(delta.x) );
				relativeRot = rotQuat * relativeRot;
				relativeRot.Normalize();
			}

			// Vertical
			{
				Quaternion rotQuat ( relativeRot.GetRight(), 0.25f * DEG2RAD(delta.y) );
				relativeRot = rotQuat * relativeRot;
				relativeRot.Normalize();
			}

			mObject->SetRelativeRotation(relativeRot);
			return true;
		}
		return false;
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
	bool MouseMove (const Vector2i& pos, const Vector2i& delta);
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow(8);
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	Script::Register<OSRotateLight>();
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
	if (*mCore << "Config/Dev13.txt")
	{
		while (mCore->Update());
		//*mCore >> "Config/Dev13.txt";
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