//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev10: Phil's Torch
//------------------------------------------------------------------------------------------------------------
// The torch on display
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image
//============================================================================================================

#include "../Include/_All.h"
using namespace R5;

R5::Random randomGen;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
	Scene			mScene;
	DebugCamera*	mCam;

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
	mCore		= new Core(mWin, mGraphics, 0, mScene);

	// Register the new fire and smoke emitters
	Object::Register<FireEmitter>();
	Object::Register<SmokeEmitter>();
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
		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		ModelInstance* ins = mScene.FindObject<ModelInstance>("Peasant");

		if (ins != 0)
		{
			Model* model = ins->GetModel();
			model->PlayAnimation("Walk");
			model->PlayAnimation("Torch");
		}

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			while (mCore->Update());
		}
	}
	//*mCore >> "Config/Dev10.txt";
}

//============================================================================================================
// Scene::Draw()
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	Deferred::DrawResult result = mScene.DrawAllDeferred(0, 0);
	mScene.DrawAllForward(false);
	PostProcess::Bloom(mGraphics, result.mColor, 1.0f);
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