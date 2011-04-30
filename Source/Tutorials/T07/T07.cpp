//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 07: Deferred Lighting, HDR and Post-processing effects
//------------------------------------------------------------------------------------------------------------
// Seventh tutorial shows how to set up R5 to use the deferred shading approach while also adding HDR Bloom.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, Render
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
// We need to change the material once again, adding a deferred rendering method
//============================================================================================================

void TestApp::Run()
{
	// Load everything that was in the previous tutorial -- camera, light, model, skybox.
	if (*mCore << "Config/T07.txt")
	{
		// Since all our data comes from the configuration file, we don't need to change the model's material
		// here in the code. What we do need to do is change "OSDrawForward" script attached to the camera to
		// "OSDrawDeferred", and to add the "Deferred" technique to the Chromatic material. That's it!
		// That's all it takes to get your model rendering using the deferred approach. Try modifying the
		// configuration file and switching "OSDrawDeferred" with "OSDrawForward". :)

		// Enter the message processing loop
		while (mCore->Update());

		//*mCore >> "Config/T07.txt";
	}
}

//============================================================================================================
// Application entry point hasn't changed
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