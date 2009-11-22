//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Tutorial 02: Resource Files
//------------------------------------------------------------------------------------------------------------
// This tutorial shows an alternate way of initializing your application by storing the init information in
// an external resource file that can then be shared across different applications. The resource files can
// be used to store all kinds of information: from window creation to the full UI layout and complete scene
// hierarchy. In this tutorial we will cover the basics: how to load and save your application.
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
// Load the application's configuration, set up listeners and enter the message processing loop
//============================================================================================================

void TestApp::Run()
{
	// First thing we want to do is load the resource file. In case you're wondering, this resource
	// file was created by simply saving the previous tutorial with the opposite call (>>).
	// See the end of this function for details.

	if (*mCore << "Config/T02.txt")
	{
		// At this point our window has been created. Our camera has also been loaded, so now we just find it:
		mCam = FindObject<DebugCamera>(mCore->GetScene(), "Default Camera");

		// Register our listeners and enter the message processing loop, same as in the previous tutorial
		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update());

		// You can also save your application here if you want, and doing so will make all the changes you
		// make persistent. The next time you start up this application the camera will be where you left it.
		//*mCore >> "Config/T02.txt";
	}
}

//============================================================================================================
// The OnDraw function hasn't changed since the previous tutorial
//============================================================================================================

void TestApp::OnDraw()
{
	mCore->BeginFrame();
	mCore->CullScene(mCam);
	mCore->PrepareScene();
	mGraphics->Draw( IGraphics::Drawable::Grid );
	mCore->EndFrame();
	Thread::Sleep(1);
}

//============================================================================================================
// Application entry point now needs to navigate to the resources folder
//============================================================================================================

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	// Mac OS launches all programs inside the application package folder
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	// We want to navigate to the resources folder before starting the application
	System::SetCurrentPath("../../../Resources/");
	TestApp app;
    app.Run();
	return 0;
}