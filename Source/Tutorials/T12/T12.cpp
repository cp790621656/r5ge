//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 12: Unique Animated Models (Part 2/2)
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to create unique models via code.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Image
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================
// Main Class
//============================================================================================================

class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	Core*		mCore;

public:

	TestApp();
	~TestApp();

	ModelInstance* CreateNewInstance (ModelTemplate* temp, const String& modelName, const String& instanceName);

	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mCore		= new Core(mWin, mGraphics);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================
// Just like in the previous tutorial we need to create a new model for each uniquely animated instance.
// This function does just that: it takes a model template and creates a new model, then creates a new
// model instance using that model. All that's left is to position this instance somewhere in the world.
//============================================================================================================

ModelInstance* TestApp::CreateNewInstance (ModelTemplate* temp, const String& modelName, const String& instanceName)
{
	// Create a new model using the template
	Model* model = mCore->GetModel(modelName);
	model->SetSource(temp);

	// Create a new instance of the model above
	ModelInstance* inst = mCore->GetRoot()->AddObject<ModelInstance>(instanceName);
	inst->SetModel(model);
	inst->AddScript<OSPlayIdleAnimations>();
	return inst;
}

//============================================================================================================
// In the Run function we want to create the new model instances before entering the game loop
//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/T12.txt")
	{
		// Create a model template
		ModelTemplate* temp = mCore->GetModelTemplate("Models/peasant.r5a");

		// Create the 3 instances and position them, matching the previous tutorial's setup
		CreateNewInstance(temp, "Peasant 0", "First")->SetRelativePosition(Vector3f(0, 3, 0));
		CreateNewInstance(temp, "Peasant 1", "Second")->SetRelativePosition(Vector3f(1, -2, 0));
		CreateNewInstance(temp, "Peasant 2", "Third")->SetRelativePosition(Vector3f(-2, 1, 0));

		while (mCore->Update());

		//*mCore >> "Config/T12.txt";
	}
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