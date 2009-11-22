//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Tutorial 01: Window Creation
//------------------------------------------------------------------------------------------------------------
// This tutorial illustrates how to create a simple OpenGL window using R5,
// as well as setup and control the camera with a minimal amount of effort.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
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
	mCore		= new Core(mWin, mGraphics, 0);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}


//============================================================================================================
// Create the window, set up the camera, bind listener callbacks and enter the message processing loop
//============================================================================================================

void TestApp::Run()
{
	// Create the window that will be used by the application
	mWin->Create("Tutorial 1: Window Creation", 100, 100, 900, 600);

	// Add a new camera to the scene: it's added at (0, 0, 0)
	mCam = AddObject<DebugCamera>(mCore->GetScene(), "Default Camera");

	// Set the rotation to have an isometric look
	mCam->SetRelativeRotation(Vector3f(1, 1, -1));

	// Move the camera back 10 units
	mCam->SetDolly(Vector3f(0.0f, 10.0f, 20.0f));

	// Register our draw function -- it will be called every frame unless the window is minimized
	mCore->SetListener( bind(&TestApp::OnDraw, this) );

	// We want all mouse movement and scroll events to go directly to the camera
	mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
	mCore->SetListener( bind(&Camera::OnScroll, mCam) );

	// Enter the message processing loop
	while (mCore->Update());
}

//============================================================================================================
// Callback registered with the R5::Core, triggered when it's time to draw the scene
//============================================================================================================

void TestApp::OnDraw()
{
	// Begin the drawing process
	mCore->BeginFrame();

	// Cull the scene using our camera's perspective
	mCore->CullScene(mCam);

	// Prepare to draw to the screen (when no target is specified it means we want the screen)
	mCore->PrepareScene();

	// Draw a simple 20x20 grid centered at (0, 0, 0)
	mGraphics->Draw( IGraphics::Drawable::Grid );

	// Finish the drawing process
	mCore->EndFrame();

	// Sleep for a short while, letting other threads run
	Thread::Sleep(1);
}

//============================================================================================================
// Application entry point
//============================================================================================================

R5_MAIN_FUNCTION
{
	TestApp app;
    app.Run();
	return 0;
}