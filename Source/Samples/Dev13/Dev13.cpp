//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
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

class TestApp : Thread::Lockable
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mCamScene;
	Camera*			mCam;
	Object*			mLight;

public:

	TestApp();
	~TestApp();
	void Run();
	bool MouseMove (const Vector2i& pos, const Vector2i& delta);
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0), mLight(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mCamScene);
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
		mCam = mCamScene.FindObject<Camera>("Default Camera");
		mLight = mCamScene.FindObject<Object>("First Light");

		if (mCam != 0 && mLight != 0)
		{
			// Set the listener callbacks
			// TODO: Why have this SetListener stuff for the camera here? Create a generic camera script,
			// and have this script set its own listener automatically.
			mCore->SetListener( bind(&TestApp::MouseMove, this) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			// Update and draw the scene
			while (mCore->Update()) {}

			//*mCore >> "Config/Dev13.txt";
		}
	}
}

//============================================================================================================

bool TestApp::MouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mCore->IsKeyDown(Key::L))
	{
		Quaternion relativeRot (mLight->GetRelativeRotation());

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

		mLight->SetRelativeRotation(relativeRot);
	}
	else
	{
		mCam->MouseMove(pos, delta);
	}
	return true;
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