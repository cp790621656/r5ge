//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev10: Animations
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image
//============================================================================================================

#include "../Include/_All.h"
using namespace R5;

R5::Random randomGen;

//============================================================================================================
// Script that adds torchlight-like flickering to the point light it's attached to
//============================================================================================================

class Torchlight : public Script
{
	PointLight* mLight;

public:

	R5_DECLARE_INHERITED_CLASS("Torchlight", Torchlight, Script, Script);

	virtual void OnInit()
	{
		mLight = R5_CAST(PointLight, mObject);
		if (mLight == 0) DestroySelf();
	}

	virtual void OnUpdate()
	{
		float time = Time::GetTime();
		float brightness = (float)sin(25.58213 * time) + (float)sin(9.4624 * time);
		brightness *= 0.25f;
		brightness += 0.5f;
		mLight->SetBrightness(0.75f + 0.25f * brightness);
	}
};

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
	Model*			mModel;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
	uint OnKey (const Vector2i& pos, byte key, bool isDown);
};

//============================================================================================================

TestApp::TestApp() : mModel(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mCore		= new Core(mWin, mGraphics);

	// Register the new fire and smoke emitters
	Object::Register<FireEmitter>();
	Object::Register<SmokeEmitter>();
	Script::Register<Torchlight>();
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/Dev10.txt")
	{
		mCore->AddOnKey(bind(&TestApp::OnKey, this));
		while (mCore->Update());
	}
}

//============================================================================================================
// React to key events
//============================================================================================================

uint TestApp::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	if (mModel == 0)
	{
		ModelInstance* ins = mCore->GetRoot()->FindObject<ModelInstance>("Peasant");
		if (ins != 0) mModel = ins->GetModel();
	}

	if (mModel != 0 && !isDown)
	{
		if		(key == Key::One)	mModel->PlayAnimation("Combat: Attack - Feint");
		else if (key == Key::Two)	mModel->PlayAnimation("Combat: Attack - Stab");
		else if (key == Key::Three) mModel->PlayAnimation("Combat: Attack - Swing");
		else if (key == Key::Four)	mModel->PlayAnimation("Combat: Attack - Dodge Combo");
		else if (key == Key::Five)	mModel->PlayAnimation("Combat: Attack - Slash Combo");
		else if (key == Key::Six)	mModel->PlayAnimation("Combat: Attack - Dodge Combo");
		else if (key == Key::Q)		mModel->PlayAnimation("Combat: Dodge");
		else if (key == Key::W)		mModel->PlayAnimation("Combat: Block");
		else if (key == Key::Grave)	mModel->PlayAnimation("Combat: Hit");
		else if (key == Key::Escape) mWin->Close();
		return EventDispatcher::EventResponse::Handled;
	}
	return EventDispatcher::EventResponse::NotHandled;
}

//============================================================================================================
// Application entry point
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