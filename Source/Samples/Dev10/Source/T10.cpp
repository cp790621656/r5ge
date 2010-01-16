//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 05: Lights, Models and Animations
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to set up basic lighting, load and animate a model.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image
//============================================================================================================

#include "../Include/_All.h"
using namespace R5;

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
// Creates a window, sets the camera, adds a light, and creates/animates the model.
//============================================================================================================

void TestApp::Run()
{
	// Create the window, Tutorial 1 style
	mWin->Create("Tutorial 5: Lights, models, and animations", 100, 100, 900, 600);

	// Add and position the camera -- future tutorials will be setting
	// this up in the resource files, so I hope you understand what's going on!
	mCam = mScene.AddObject<DebugCamera>("Default Camera");
	mCam->SetRelativePosition( Vector3f(0.0f, 0.0f, 6.0f) );
	mCam->SetRelativeRotation( Vector3f(1.0f, -5.0f, -1.0f) );
	mCam->SetDolly( Vector3f(0.0f, 16.0f, 30.0f) );

	

	Model* model = mCore->GetModel("First Model", true);
	model->Load("Models/tourch.r5a");
	model->PlayAnimation("Run");
	ModelInstance* instance = mScene.AddObject<ModelInstance>("First Instance");
	instance->SetModel(model);

	mCore->SetListener( bind(&TestApp::OnDraw, this) );
	mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
	mCore->SetListener( bind(&Camera::OnScroll, mCam) );

	while (mCore->Update());
}

//============================================================================================================
// We add a new function call here: Scene::Draw()
//============================================================================================================

void TestApp::OnDraw()
{
	// Cull our scene like before
	mScene.Cull(mCam);

	// Draw our scene using all forward rendering techniques. This function exists for convenience
	// reasons, and it automatically changes projection back to perspective and clears the screen.
	mScene.DrawAllForward();

	// Add the grid at the end, just to show that we can still do manual rendering afterwards.
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