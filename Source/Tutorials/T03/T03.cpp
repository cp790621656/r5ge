//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 03: User Interface (Part 1)
//------------------------------------------------------------------------------------------------------------
// This tutorial covers the basics of the UI: loading skins, creating widgets, etc.
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
	UILabel*		mLabel;

public:

	TestApp();
	~TestApp();
	void Run();
	void CreateWindow();
	void OnSliderChange(UIWidget* widget);
};

//============================================================================================================

TestApp::TestApp() : mLabel(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
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
	if (*mCore << "Config/T03.txt")
	{
		// Create a simple UI window via code
		CreateWindow();

		// Enter the message processing loop
		while (mCore->Update());

		//*mCore >> "Config/T03.txt";
	}
}

//============================================================================================================
// Creates a simple UI window with a few widgets inside
//============================================================================================================

void TestApp::CreateWindow()
{
	// Add a new UI widget: a simple window. R5::UI is optimized to "collect" all widgets into as few draw
	// calls as possible, dividing them by parent frames. Each frame creates a new set of draw buffers,
	// and each frame can be further refined by specifying different layers for widgets, which can also
	// be used to ensure that certain widgets show up on top. Everything inside the frame is drawn BY that
	// frame. UI's "Window" class is derived from "Frame", so it has the ability to render itself in
	// addition to all of its children.

	UIWindow* win = mUI->AddWidget<UIWindow>("First Window");

	// All widgets have a region which is defined by 4 relative and absolute coordinate pairs.
	// Relative coordinates are typically in the 0-1 range, 0 being the left/top, and 1 being
	// the bottom/right dimensions of the parent area. As such, a centered widget would have
	// relative coordinates based on '0.5'. Absolute coordinates offset the calculated relative
	// values. For example, setting the Left side of the Region to (0.5, -100) would position
	// the widget at 100 pixels left of the center. As another example, if you wanted to anchor
	// some widget to the right side, simply set its coordinates to be based on relative value
	// of 1, and an a negative absolute value, for example -50 to be 50 pixels to the left of 1.

	// Here we use the convenience function to set all 4 coordinates (left, top, right, bottom)
	// by specifying the top-left in absolute coordinates, followed by width and height.
	win->GetRegion().SetRect(100, 100, 300, 200);

	// Set the window title's text
	win->SetText("Hello World!");

	// Set an optional tooltip that will be displayed when the mouse hovers over the window.
	// Note that you can easily embed colors in all strings used by the UI.
	win->SetTooltip("Tutorial 3 window says [FFAA33]Hello!");

	// By default, the window doesn't have a title bar. Let's change that.
	win->SetTitlebarHeight(22);

	// Since we created this widget programmatically, let's not bother saving it.
	win->SetSerializable(false);

	// Now we want to add a slider to the window. Note that instead of 'mUI' we use 'win'.
	{
		UISlider* sld = win->AddWidget<UISlider>("First Slider");

		// We want to listen to value changing events, so that when we drag the slider our
		// callback gets called. We can do that by attaching a listener script to the widget.
		USEventListener* listener = sld->AddScript<USEventListener>();
		listener->SetOnValueChange( bind(&TestApp::OnSliderChange, this) );

		// The slider's region is padded 5 pixels on all sides, bottom side being fixed to the top
		UIRegion& rgn = sld->GetRegion();
		rgn.SetLeft		(0.0f,  5.0f);
		rgn.SetRight	(1.0f, -5.0f);
		rgn.SetTop		(0.0f,  5.0f);
		rgn.SetBottom	(0.0f, 25.0f);

		// Add a label on top of the slider
		{
			mLabel = sld->AddWidget<UILabel>("Slider Value");

			// We want to center the text inside the label
			mLabel->SetAlignment( UILabel::Alignment::Center );

			// In order to make text easier to read on both bright and dark backgrounds, it may be
			// a good idea to have it automatically drop a dark shadow behind it.
			mLabel->SetShadowColor(Color4ub(0, 0, 0, 175));

			// Note that we don't set the label's region at all. This means the label will use the full
			// dimensions of the parent. The problem with this is that since the label covers the slider
			// fully, it will intercept all of the mouse events. We don't want that -- we want the slider
			// to get the mouse events. Solution? Tell the label to ignore all events.
			mLabel->SetEventHandling( UIWidget::EventHandling::None );

			// The layer should be higher so it's drawn on top of the slider. By default the layer is 0,
			// and depending on which texture gets created first, the slider may be drawn *after* the
			// label's text. Setting the layer to '1' prevents that behavior. Note that this feature
			// should not be overused too much as each new layer creates a new draw call. Widgets on
			// the same layer using the same texture will be drawn in a single draw call. For example,
			// 100 different labels all using the default font added to one window? Single draw call.
			// Likewise if a widget will be changing frequently (such as a label that's updated every
			// single frame -- FPS?) -- you might want to place it on a different layer than widgets
			// that won't be updated frequently, just so that the infrequent widgets won't get polled
			// to fill the draw buffer alongside the frequently updated one.

			mLabel->SetLayer(1);
		}

		// Set the slider's value. This will also cause the callback we set above to trigger,
		// which will in turn set the label's text as per our OnSliderChange function below.
		sld->SetValue(0.25f);
	}
}

//============================================================================================================
// Callback triggered when the slider's value changes
//============================================================================================================

void TestApp::OnSliderChange (UIWidget* widget)
{
	// The passed area is actually a slider
	UISlider* sld = R5_CAST(UISlider, widget);
	
	// Set the label's text, mirroring the slider's value
	if (sld != 0 && mLabel != 0) mLabel->SetText( String("Value: %.2f", sld->GetValue()) );
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