//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
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
// You may notice that our class is visibly shorter than that of the previous tutorial. The scene is no longer
// present, and neither is the camera. This is because we will render the scene using a built-in script.
//============================================================================================================

class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	Core*		mCore;

public:

	TestApp();
	~TestApp();
	void Run();
};

//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow(8);
	mGraphics	= new GLGraphics();
	mCore		= new Core(mWin, mGraphics);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================
// Load the application's configuration, set up the custom OnDraw and enter the message processing loop
//============================================================================================================

void TestApp::Run()
{
	// First thing we want to do is load the resource file. In case you're wondering, this resource
	// file was created by simply saving the previous tutorial with the opposite call (>>) on the Core.
	if (*mCore << "Config/T02.txt")
	{
		// Enter the message processing loop
		while (mCore->Update());

		// You can also save your application here if you want, and doing so will make all the changes you
		// make persistent. The next time you start up this application the camera will be where you left it.
		//*mCore >> "Config/T02.txt";
	}
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