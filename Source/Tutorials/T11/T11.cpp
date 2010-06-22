//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 11: Unique Animated Models (Part 1/2)
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to create models that are uniquely animated.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Image
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
using namespace R5;

//============================================================================================================
// You may have noticed that in the previous tutorial all models were animated exactly the same. This was
// because all of them were created using the the same exact Model Template. Animations happen on models, but
// we can have multiple instances of the same model in the scene, which is exactly what we had in Tutorial 10.
//
// You may wonder why that's so, but consider a large forest of 1000 trees. You may animate the entire forest
// using unique animations for each tree, but that means 1000 unique animations. Sure, it will look great and
// completely random, but the performance will be terrible at best. The same "uniquely animated" look can be
// easily faked by only uniquely animating 4-6 trees, then simply instancing them randomly within the scene.
// Doing so will achieve the same visual effect at a fraction of the cost.
//
// Of course there comes a time when models need to be animated uniquely: characters, for example.
// The main (and only) difference is how we create the models, which happens in the configuration file.
// Rather than specifying the model file directly for each Model Instance in our scene, we now specify a
// model template instead, like so:
//============================================================================================================
//	Scene
//	{
//		Model Instance = "First"
//		{
//			Position = 0 3 0
//			Model = "Peasant 0"
//		}
//	}
//============================================================================================================
// Of course we now have to create this template first, using either another template, or the model file:
//============================================================================================================
//	Core
//	{
//		Model Template = "Peasant 0"
//		{
//			Source = "Models/peasant.r5a"
//		}
//	}
//============================================================================================================
// One cool thing worth mentioning is how templates can be created based on other templates, which is what
// we're doing in the configuration file. We can also specify an entire TreeNode to be passed to the
// OnSerializeFrom function of each Model Instance that gets created using this template, allowing us to
// "cache" such data as the default position, rotation, which scripts will be automatically attached, etc.
// That's exactly what we're doing in the configuration file when we attach "OSPlayIdleAnimations".
//============================================================================================================
//	Core
//	{
//		Model Template = "Peasant"
//		{
//			Source = "Models/peasant.r5a"
//			
//			OnSerialize
//			{
//				Script = "OSPlayIdleAnimations"
//			}
//		}
//		
//		Model Template = "Peasant 0"
//		{
//			Source = "Peasant"
//		}
//		
//		Model Template = "Peasant 1"
//		{
//			Source = "Peasant"
//		}
//		
//		Model Template = "Peasant 2"
//		{
//			Source = "Peasant"
//		}
//	}
//============================================================================================================
// Of course you may want to create unique models via code instead of the configuration files. We'll cover
// how to do that in the next tutorial.
//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;

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
// In the Run function we only want to load the configuration files and enter the update loop
//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/T11.txt")
	{
		while (mCore->Update());
		//*mCore >> "Config/T11.txt";
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