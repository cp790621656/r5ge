#include "../Include/Dev1.h"
using namespace R5;

//------------------------------------------------------------------------------------------------------------

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	mUI->SetOnValueChange ("First Slider",	bind(&TestApp::OnChangeDelegate,	this));
	mUI->SetOnValueChange ("Range",			bind(&TestApp::OnRangeChange,		this));
	mUI->SetOnValueChange ("Power",			bind(&TestApp::OnPowerChange,		this));
	mUI->SetOnValueChange ("Brightness",	bind(&TestApp::OnBrightnessChange,	this));
	mUI->SetOnStateChange ("First Button",	bind(&TestApp::OnButtonStateChange,	this));
	mUI->SetOnStateChange ("Second Button",	bind(&TestApp::OnButtonStateChange,	this));
	mUI->SetOnStateChange ("Third Button",	bind(&TestApp::OnButtonStateChange,	this));
}

//------------------------------------------------------------------------------------------------------------

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//------------------------------------------------------------------------------------------------------------

void TestApp::Run()
{
    if (*mCore << "Config/Dev1.txt")
	{
		mCam = mScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&TestApp::OnKeyPress, this) );
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			while (mCore->Update());

			//*mCore >> "Config/Dev1.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllForward();
}

//------------------------------------------------------------------------------------------------------------

bool TestApp::OnKeyPress(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
		}
		else if ( key == Key::H && isDown )
		{
			UIWindow* window = mUI->FindWidget<UIWindow>("First Window");
			if (window) window->Show();
		}
		else if ( key == Key::F5 )
		{
			mWin->SetStyle( mWin->GetStyle() == IWindow::Style::FullScreen ?
				IWindow::Style::Normal : IWindow::Style::FullScreen);
		}
		else if ( key == Key::F6 )
		{
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
		}
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------

void TestApp::OnChangeDelegate (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val		  = slider->GetValue();
		float redFactor   = Float::Clamp(2.0f - val * 2.0f, 0.0f, 1.0f);
		float greenFactor = Float::Clamp(       val * 2.0f, 0.0f, 1.0f);

		slider->SetColor( Color3f(redFactor, greenFactor, greenFactor * 0.15f) );
	}
}

//------------------------------------------------------------------------------------------------------------

void TestApp::OnBrightnessChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val = slider->GetValue() * 4.0f;

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.3f", val) );

		PointLight* light = mScene.FindObject<PointLight>("First Light");
		if (light) light->SetBrightness(val);
	}
}

//------------------------------------------------------------------------------------------------------------

void TestApp::OnRangeChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val = slider->GetValue() * 40.0f;

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );

		PointLight* light = mScene.FindObject<PointLight>("First Light");
		if (light) light->SetRange(val);
	}
}

//------------------------------------------------------------------------------------------------------------

void TestApp::OnPowerChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val = 1.0f + slider->GetValue() * 3.0f;

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.3f", val) );

		PointLight* light = mScene.FindObject<PointLight>("First Light");
		if (light) light->SetPower(val);
	}
}

//------------------------------------------------------------------------------------------------------------

void TestApp::OnButtonStateChange (UIWidget* widget, uint state, bool isSet)
{
	UIButton* btn = R5_CAST(UIButton, widget);

	if (btn)
	{
		String str;

		if (btn->GetState() & UIButton::State::Enabled)		str = "Enabled";
		else												str = "Disabled";
		if (btn->GetState() & UIButton::State::Pressed)		str << ", Pressed";
		if (btn->GetState() & UIButton::State::Highlighted)	str << ", Highlighted";

		str << " - ";
		str << Time::GetTime();
		btn->SetText(str);
	}
}

//------------------------------------------------------------------------------------------------------------

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