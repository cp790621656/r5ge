//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 04: User Interface (Part 2)
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to do the same thing as the previous tutorial, but by loading a resource file
// containing the UI Layout information. The slider is also driven by a custom script rather than a callback.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================
// UI Script that can be attached to a slider to update its label's text
//------------------------------------------------------------------------------------------------------------
// Instead of attaching a listener script to the slider that was forwarding the value change event
// to our function delegate, we'll attach a custom script instead that will not only update the slider's
// label, but will actually create it to begin with. Then all we have to do is attach it to our slider.
//============================================================================================================

class SliderCaption : public UIScript
{
	UISlider*	mSlider;
	UILabel*	mLabel;

public:

	SliderCaption() : mSlider(0), mLabel(0) {}

	// Declare this script type
	R5_DECLARE_INHERITED_CLASS("SliderCaption", SliderCaption, UIScript, UIScript);

	// Init function gets called when the script is first being initialized
	virtual void Init()
	{
		// This script only works if it's attached to a slider
		mSlider = R5_CAST(UISlider, mWidget);

		if (mSlider != 0)
		{
			// Add a label to the slider and mark it as not serializable, so it's not saved out
			mLabel = mWidget->AddWidget<UILabel>(mWidget->GetName() + " value");
			mLabel->SetSerializable(false);
			mLabel->SetReceivesEvents(false);
			mLabel->SetAlignment(UILabel::Alignment::Center);
		}
		else
		{
			// This script was not attached to a slider -- destroy it
			DestroySelf();
		}
	}

	// Overwrite the callback function that gets triggered when the owner's value changes
	virtual void OnValueChange()
	{
		// Get the slider's value and set the child label's text
		mLabel->SetText( String("Value: %.2f", mSlider->GetValue()) );
	}
};

//============================================================================================================
// Main class for our tutorial
//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
	UI*				mUI;
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
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	// Register our UI script so that it can be created via the UIWidget::AddScript<> template
	UIScript::Register<SliderCaption>();
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
	// The bulk of the config for this tutorial takes place inside "T04.txt" file. Previous tutorial was
	// simply saved to a file, and this file is used by this tutorial. You can uncomment the ">>" line at the
	// end of this function to enable persistent behavior (slider's value will be saved for next time).
	// Note though that the label that was on top of the slider has been removed for this tutorial as it's
	// now being added by the script we're attaching to the slider.

	if ((*mCore << "Config/T04.txt") && (*mCore << "Config/Default UI Skin.txt"))
	{
		// Find the UISlider defined inside the "T04" resource file
		UISlider* slider = mUI->FindWidget<UISlider>("First Slider");

		// Add a script to the slider that we've created above. This will in turn create a label on top
		// of our slider and will later trigger the script's OnValueChange function, updating tha label's value.
		slider->AddScript<SliderCaption>();

		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Object::MouseMove, mCam) );
		mCore->SetListener( bind(&Object::Scroll, mCam) );

		while (mCore->Update());

		//*mCore >> "Config/T04.txt";
	}
}

//============================================================================================================
// The OnDraw function hasn't changed
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mGraphics->Clear();
	mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
	mGraphics->Draw( IGraphics::Drawable::Grid );
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