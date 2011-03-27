//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
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
// We need to change the material once again, adding a deferred rendering method
//============================================================================================================

void TestApp::Run()
{
	// Load everything that was in the previous tutorial -- camera, light, model, skybox.
	if (*mCore << "Config/T07.txt")
	{
		// Find the model and play the "Run" animation
		Model* model = mCore->GetModel("Models/peasant.r5a");
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
		method->mShader = shader;

		// Find and reuse the skybox created in the previous tutorial
		method->SetTexture(0, mGraphics->GetTexture("Skybox"));

		// That's all there is to it, with one exception: OSDrawForward script attached to the camera has
		// been replaced with OSDrawDeferred. Deferred method uses a combination of deferred techniques
		// followed by transparent and blended objects rendered via forward rendering (such as lens flare
		// effects, particles, glow/glare billboards, etc). You can pass additional parameters to the
		// OSDrawDeferred function such as "Bloom" to adjust the amount of bloom, "Focal Range" to change
		// where and how depth-of-field effect happens, and "SSAO" to add screen-space ambient occlusion.
		// Note that in order to see the skybox, the background color needs to have an alpha of 0.

		// Enter the message processing loop
		while (mCore->Update());

		//*mCore >> "Config/T07.txt";
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