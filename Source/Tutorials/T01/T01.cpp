//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
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
	Scene			mScene;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp() : mCam(0)
{
	// Note that the constructor we're currently using for the Core is a convenience constructor. The last
	// parameter (scene) is optional. All it does inside is sets the scene's root to be the root of the
	// object hierarchy tree residing inside the Core. R5 allows you to have more than one scene using
	// different (or the same!) parts of the object tree, making it possible to create efficient animated
	// character thumbnails, object preview windows, and much more.

	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mCore		= new Core(mWin, mGraphics, 0, 0, mScene);
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
	mCam = mScene.AddObject<DebugCamera>("Default Camera");

	// Set the rotation to have an isometric look
	mCam->SetRelativeRotation(Vector3f(1, 1, -1));

	// Move the camera back 10 units
	mCam->SetDolly(Vector3f(0.0f, 10.0f, 20.0f));

	// Note that the debug camera is a convenience camera. It registers itself as a listener for mouse move
	// and key events so you don't have to. With any other camera you will have to create a script to handle
	// how it reacts to various keys -- but we'll cover scripts in the next tutorial.

	// Register our draw function -- it will be called every frame unless the window is minimized.
	// By default the priority of the function is 1000, and higher priority callbacks are called. first.
	mCore->AddOnDraw( bind(&TestApp::OnDraw, this) );

	// Note that it's easier to simply add "OSDrawForward" or "OSDrawDeferred" script to the camera
	// and that will take care of drawing the scene for you using forward or deferred rendering, respectively.
	// Doing that won't let you manually draw things however, such as the grid we're adding below.

	// Enter the message processing loop
	while (mCore->Update());
}

//============================================================================================================
// Callback registered with the R5::Core, triggered when it's time to draw the scene
//============================================================================================================

void TestApp::OnDraw()
{
	// Cull the scene from our camera's perspective
	mScene.Cull(mCam);

	// Clear the screen
	mGraphics->Clear();

	// Draw a simple 20x20 grid centered at (0, 0, 0)
	mGraphics->Draw( IGraphics::Drawable::Grid );
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