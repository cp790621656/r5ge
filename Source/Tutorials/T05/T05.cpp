//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Tutorial 05: Lights, Models and Animations
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to set up basic lighting, load and animate a model.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
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
	mCam = AddObject<DebugCamera>(mScene, "Default Camera");
	mCam->SetRelativePosition( Vector3f(0.0f, 0.0f, 6.0f) );
	mCam->SetRelativeRotation( Vector3f(1.0f, -5.0f, -1.0f) );
	mCam->SetDolly( Vector3f(0.0f, 16.0f, 30.0f) );

	// Let's attach a directional light source to the camera just so that we can see the scene.
	DirectionalLight* light = AddObject<DirectionalLight>(mCam, "First Light");

	// Faint ambient color
	light->SetAmbient( Color3f(0.15f, 0.15f, 0.15f) );

	// Make the light's diffuse color oversaturated, adding vibrance to the scene
	light->SetDiffuse( Color3f(1.25f, 1.25f, 1.25f) );

	// Here we create a new model template. Note that creating a model will NOT place it into the scene.
	// In R5 all models are instanced in the world, allowing you to create entire crowds of animated
	// characters using only a few actual animated models. This saves the memory and CPU cycles. Big time.
	// Think of an entire forest of 1000 trees: you can create an entire forest by individually animating
	// each tree, sure. But wouldn't it make more sense to individually animate only say... 5 trees,
	// then randomly place them all over the forest, mixing them so that it *seems* like the entire
	// forest is uniquely animated? This way you're only paying for 5 animations, not 1000. R5 allows
	// you to do just that. Animate and skin once, render multiple times.

	Model* model = mCore->GetModel("First Model", true);

	// Load the peasant model and play the "Run" animation defined inside the model file.
	// The model is saved in ASCII format on purpose -- you can open it up and see what's inside.

	model->Load("Models/peasant.r5a");
	model->PlayAnimation("Run");

	// Now comes the instancing part. We want to create an instance of this model inside our world.
	// In order to keep it simple we just add it to the scene as-is, and won't even bother changing
	// its default orientation.

	ModelInstance* instance = AddObject<ModelInstance>(mScene, "First Instance");
	instance->SetModel(model);

	// And that's all there is to it! You can play additional animations by using the Model::PlayAnimation
	// function, but keep in mind that "Run" animation we played above is a high layer animation, which
	// means that it covers lower layer animations (which includes all idle animations). In addition,
	// animations that are played on the same layer replace each other. As an example, both "Run" and "Walk"
	// animations sit on layer 2, so when one is played, it gradually replaces the other as it fades in.

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
	mScene.Cull(mCam);
	mGraphics->Clear();
	mGraphics->Draw( IGraphics::Drawable::Grid );
	mScene.Draw();	// <-- NEW! Draw the scene using all normal forward rendering techniques.
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