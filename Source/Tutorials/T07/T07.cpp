//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Tutorial 07: Deferred Shading, HDR and Post-processing effects
//------------------------------------------------------------------------------------------------------------
// Seventh tutorial shows how to set up R5 to use the deferred shading approach while also adding HDR Bloom.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, Render
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
// We need to change the material once again, adding a deferred rendering method
//============================================================================================================

void TestApp::Run()
{
	// Load everything that was in the previous tutorial -- camera, light, model, skybox.
	if (*mCore << "Config/T07.txt")
	{
		// Find the model and play the "Run" animation
		Model* model = mCore->GetModel("First Model");
		model->PlayAnimation("Run");

		// Retrieve the material created in the previous tutorial
		IMaterial* mat = mGraphics->GetMaterial("Chromatic");

		// The material we created in the previous tutorial has only one rendering technique associated with it:
		// the "Opaque" one as you might recall. Unfortunately it's a forward rendering technique, meaning
		// that if we were to simply try to draw the model as-is using our deferred approach only the skybox
		// would be visible. The model would be missing. The reason behind it is quite simple and you may have
		// already figured it out: deferred rendering uses the "Deferred" technique, but since the material
		// used by our model doesn't have the Rendering Method for that technique, it simply won't show up.
		// Let's go ahead and add this new rendering method to our material.

		ITechnique* deferred = mGraphics->GetTechnique("Deferred");

		// Add a new rendering method to our material that will be used with the deferred technique
		IMaterial::DrawMethod* method = mat->GetDrawMethod(deferred);

		// Deferred approach can't use forward rendering shaders. Let's find its deferred equivalent.
		IShader* shader = mGraphics->GetShader("Deferred/chromatic");

		// Set the shader that will be used to draw this material
		method->SetShader(shader);

		// Find and reuse the skybox created in the previous tutorial
		method->SetTexture(0, mGraphics->GetTexture("Skybox"));

		// The rest of this function hasn't changed
		mCam = FindObject<DebugCamera>(mScene, "Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update());
	}
}

//============================================================================================================
// The drawing process changes a fair bit. We will be going through the Draw library now.
//============================================================================================================

void TestApp::OnDraw()
{
	// Cull the scene like before, building the lists of visible objects and lights
	mScene.Cull(mCam);

	// Draw our scene using all default deferred rendering techniques, which includes our "Deferred"
	// technique used by the model. As a bonus we can also add a bloom post-processing effect.
	mScene.DrawAllDeferred(false, true);

	// Note that the function above is a convenience function, just like 'DrawAllForward' function we used
	// before. As such, its functionality, while perfect for small demos, can also be limiting. You are
	// more than welcome to take a look at what's happening inside it and bypass it altogether if desired.
	// In doing so you can also add additional post-processing effects, such as depth-of-field.
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