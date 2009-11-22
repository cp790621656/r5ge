#include "../Include/Dev1.h"
using namespace R5;

//------------------------------------------------------------------------------------------------------------

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI);

	mUI->SetOnValueChange ("First Slider",	bind(&TestApp::OnChangeDelegate,	this));
	mUI->SetOnValueChange ("Range",			bind(&TestApp::OnRangeChange,		this));
	mUI->SetOnValueChange ("Power",			bind(&TestApp::OnPowerChange,		this));
	mUI->SetOnValueChange ("Brightness",	bind(&TestApp::OnBrightnessChange,	this));
	mUI->SetOnValueChange ("First Button",	bind(&TestApp::OnButtonStateChange,	this));
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
		mCam = FindObject<Camera>(mCore->GetScene(), "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&TestApp::OnKey, this) );
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );

			while (mCore->Update());

			*mCore >> "Config/Dev1.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mCore->BeginFrame();
	mCore->CullScene(mCam);
	mCore->PrepareScene();
	mCore->DrawScene();
	mCore->DrawUI();
	mCore->EndFrame();
	Thread::Sleep(1);
}

//------------------------------------------------------------------------------------------------------------

bool TestApp::OnKey(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
		}
		else if ( key == Key::H && isDown )
		{
			Window* window = FindWidget<Window>(mUI, "First Window");
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

bool TestApp::OnChangeDelegate (Area* area)
{
	Slider* slider = R5_CAST(Slider, area);

	if (slider)
	{
		float val		  = slider->GetValue();
		float redFactor   = Float::Clamp(2.0f - val * 2.0f, 0.0f, 1.0f);
		float greenFactor = Float::Clamp(       val * 2.0f, 0.0f, 1.0f);

		slider->SetColor( Color3f(redFactor, greenFactor, greenFactor * 0.15f) );
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------

bool TestApp::OnBrightnessChange (Area* area)
{
	Slider* slider = R5_CAST(Slider, area);

	if (slider)
	{
		float val = slider->GetValue() * 4.0f;

		TextLine* txt = FindWidget<TextLine>(slider, slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.3f", val) );

		PointLight* light = FindObject<PointLight>(mCore->GetScene(), "First Light");
		if (light) light->SetBrightness(val);
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------

bool TestApp::OnRangeChange (Area* area)
{
	Slider* slider = R5_CAST(Slider, area);

	if (slider)
	{
		float val = slider->GetValue() * 40.0f;

		TextLine* txt = FindWidget<TextLine>(slider, slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );

		PointLight* light = FindObject<PointLight>(mCore->GetScene(), "First Light");
		if (light) light->SetRange(val);
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------

bool TestApp::OnPowerChange (Area* area)
{
	Slider* slider = R5_CAST(Slider, area);

	if (slider)
	{
		float val = 1.0f + slider->GetValue() * 3.0f;

		TextLine* txt = FindWidget<TextLine>(slider, slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.3f", val) );

		PointLight* light = FindObject<PointLight>(mCore->GetScene(), "First Light");
		if (light) light->SetPower(val);
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------

bool TestApp::OnButtonStateChange (Area* area)
{
	Button* btn = R5_CAST(Button, area);

	if (btn)
	{
		String state;

		if (btn->GetState() & Button::State::Enabled)		state = "Enabled";
		else												state = "Disabled";
		if (btn->GetState() & Button::State::Pressed)		state << ", Pressed";
		if (btn->GetState() & Button::State::Highlighted)	state << ", Highlighted";

		state << " - ";
		state << Time::GetTime();
		btn->SetText(state);
	}
	return true;
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