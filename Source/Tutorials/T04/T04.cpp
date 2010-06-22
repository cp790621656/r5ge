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

class USSliderCaption : public UIScript
{
	UISlider*	mSlider;
	UILabel*	mLabel;

	USSliderCaption() : mSlider(0), mLabel(0) {}

public:

	// Declare this script type
	R5_DECLARE_INHERITED_CLASS("USSliderCaption", USSliderCaption, UIScript, UIScript);

	// Init function gets called when the script is first being initialized
	virtual void OnInit()
	{
		// This script only works if it's attached to a slider
		mSlider = R5_CAST(UISlider, mWidget);

		if (mSlider != 0)
		{
			// Add a label to the slider and mark it as not serializable, so it's not saved out
			mLabel = mWidget->AddWidget<UILabel>(mWidget->GetName() + " value");
			mLabel->SetSerializable(false);
			mLabel->SetEventHandling( UIWidget::EventHandling::None );
			mLabel->SetAlignment(UILabel::Alignment::Center);
			mLabel->SetShadow(true);
			mLabel->SetLayer(1);
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
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	// Register our UI script so that it can be created via the UIWidget::AddScript<> template
	UIScript::Register<USSliderCaption>();
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
	// simply saved to a file, and this file is used by this tutorial with one addition: the script above
	// (USSliderCaption) was added to the slider. You can uncomment the ">>" line at the end of this
	// function to enable persistent behavior (slider's value will be saved for next time). You can
	// also choose to add that script via code instead using the UIWidget::AddScript<> template.

	if ((*mCore << "Config/T04.txt") && (*mCore << "Config/Default UI Skin.txt"))
	{
		// Register our custom grid-drawing function
		mCore->AddOnDraw( bind(&TestApp::OnDraw, this) );

		// Enter the message processing loop
		while (mCore->Update());

		//*mCore >> "Config/T04.txt";
	}
}

//============================================================================================================
// The OnDraw function hasn't changed
//============================================================================================================

void TestApp::OnDraw()
{
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