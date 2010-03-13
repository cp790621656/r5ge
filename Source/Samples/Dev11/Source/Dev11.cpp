//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev11: Sound
//------------------------------------------------------------------------------------------------------------
// Sound testing
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Sound/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"

using namespace R5;

R5::Random randomGen;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
	Audio*			mAudio;
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
	mAudio		= new Audio();
	mCore		= new Core(mWin, mGraphics, 0, mScene, mAudio);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mAudio)		delete mAudio;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
	// Create the window that will be used by the application
	mWin->Create("Dev 11: Audio", 100, 100, 900, 600);

	// Add a new camera to the scene: it's added at (0, 0, 0)
	mCam = mScene.AddObject<DebugCamera>("Default Camera");

	// Set the rotation to have an isometric look
	mCam->SetRelativeRotation(Vector3f(1, 1, -1));

	// Move the camera back 10 units
	mCam->SetDolly(Vector3f(0.0f, 10.0f, 20.0f));

	// Register our draw function -- it will be called every frame unless the window is minimized
	mCore->SetListener( bind(&TestApp::OnDraw, this) );

	// We want all mouse movement and scroll events to go directly to the camera
	mCore->SetListener( bind(&Object::MouseMove, mCam) );
	mCore->SetListener( bind(&Object::Scroll, mCam) );

	mAudio->Play(String("Sound/cAudioTheme1.ogg"), 0, false, 1.0f);

	// Enter the message processing loop
	while (mCore->Update());
}

//============================================================================================================
// Scene::Draw()
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	Deferred::DrawResult result = mScene.DrawAllDeferred(0, 0);
	mScene.DrawAllForward(false);
	PostProcess::Bloom(mGraphics, result.mColor, 1.0f);
}

//============================================================================================================
// Application entry point
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