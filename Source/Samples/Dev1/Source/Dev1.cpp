#include "../Include/Dev1.h"
using namespace R5;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	mUI->SetOnValueChange ("First UISlider",	bind(&TestApp::OnChangeDelegate,	this));
	mUI->SetOnValueChange ("Range",				bind(&TestApp::OnRangeChange,		this));
	mUI->SetOnValueChange ("Power",				bind(&TestApp::OnPowerChange,		this));
	mUI->SetOnValueChange ("Brightness",		bind(&TestApp::OnBrightnessChange,	this));
	mUI->SetOnStateChange ("First UIButton",	bind(&TestApp::OnButtonStateChange,	this));
	mUI->SetOnStateChange ("Second UIButton",	bind(&TestApp::OnButtonStateChange,	this));
	mUI->SetOnStateChange ("Third UIButton",	bind(&TestApp::OnButtonStateChange,	this));
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

void TestApp::Run()
{
    if (*mCore << "Config/Dev1.txt")
	{
		while (mCore->Update());
		//*mCore >> "Config/Dev1.txt";
	}
}

//============================================================================================================

void TestApp::OnChangeDelegate (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val		  = slider->GetValue();
		float redFactor   = Float::Clamp(2.0f - val * 2.0f, 0.0f, 1.0f);
		float greenFactor = Float::Clamp(       val * 2.0f, 0.0f, 1.0f);

		slider->SetBackColor( Color3f(redFactor, greenFactor, greenFactor * 0.15f) );
	}
}

//============================================================================================================

void TestApp::OnBrightnessChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val = slider->GetValue() * 4.0f;

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.3f", val) );

		PointLight* light = mCore->GetRoot()->FindObject<PointLight>("First Light");
		if (light) light->SetBrightness(val);
	}
}

//============================================================================================================

void TestApp::OnRangeChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val = slider->GetValue() * 40.0f;

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );

		PointLight* light = mCore->GetRoot()->FindObject<PointLight>("First Light");
		if (light) light->SetRange(val);
	}
}

//============================================================================================================

void TestApp::OnPowerChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		float val = 1.0f + slider->GetValue() * 3.0f;

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.3f", val) );

		PointLight* light = mCore->GetRoot()->FindObject<PointLight>("First Light");
		if (light) light->SetPower(val);
	}
}

//============================================================================================================

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