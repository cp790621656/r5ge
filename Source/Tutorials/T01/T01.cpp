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
	mCore		= new Core(mWin, mGraphics, 0, mScene);
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

	// Register our draw function -- it will be called every frame unless the window is minimized
	mCore->SetListener( bind(&TestApp::OnDraw, this) );

	// We want all mouse movement and scroll events to go directly to the camera
	mCore->SetListener( bind(&Object::MouseMove, mCam) );
	mCore->SetListener( bind(&Object::Scroll, mCam) );

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

	// Activate the perspective projection
	mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );

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