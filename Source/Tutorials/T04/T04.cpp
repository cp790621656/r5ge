//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Tutorial 04: User Interface (Part 2)
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to do the same thing as the previous tutorial, but via the resource file.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
	UI*				mUI;
	DebugCamera*	mCam;
	UILabel*			mLabel;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
	bool OnSliderChange(UIArea* area);
};

//============================================================================================================

TestApp::TestApp() : mCam(0), mLabel(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI);
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
// Expanding on the previous tutorial, some UI serialization and window creation
//============================================================================================================

void TestApp::Run()
{
	// R5::UI has the ability to bind callbacks to widgets even before they get created.
	mUI->SetOnValueChange( "First Slider", bind(&TestApp::OnSliderChange, this) );

	// The bulk of the config for this tutorial takes place inside "T04.txt" file. Previous tutorial was
	// simply saved to a file, and this file is used by this tutorial. You can uncomment the ">>" line at the
	// end of this function to enable persistent behavior (slider's value will be saved for next time).

	if ((*mCore << "Config/T04.txt") && (*mCore << "Config/Default UI Skin.txt"))
	{
		// Find the UI widget defined inside the "T04" resource file
		mLabel = FindWidget<UILabel>(mUI, "Slider Value");

		mCam = FindObject<DebugCamera>(mCore->GetScene(), "Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update());

		//*mCore >> "Config/T04.txt";
	}
}

//============================================================================================================
// The draw function is identical to the one in the previous tutorial
//============================================================================================================

void TestApp::OnDraw()
{
	mCore->BeginFrame();
	mCore->CullScene(mCam);
	mCore->PrepareScene();
	mGraphics->Draw( IGraphics::Drawable::Grid );
	mCore->DrawUI();
	mCore->EndFrame();
	Thread::Sleep(1);
}

//============================================================================================================
// The callback is identical to the one in the previous tutorial
//============================================================================================================

bool TestApp::OnSliderChange (UIArea* area)
{
	UISlider* sld = R5_CAST(UISlider, area);
	if (sld != 0 && mLabel != 0) mLabel->SetText( String("Value: %.2f", sld->GetValue()) );
	return true;
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