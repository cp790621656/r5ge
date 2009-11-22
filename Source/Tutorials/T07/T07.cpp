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

#include "../../Engine/Render/Include/_All.h"
#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
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
	mCore		= new Core(mWin, mGraphics, 0);
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
		IMaterial::RenderingMethod* method = mat->GetRenderingMethod(deferred);

		// Deferred approach can't use forward rendering shaders. Let's find its deferred equivalent.
		IShader* shader = mGraphics->GetShader("Deferred/chromatic");

		// Set the shader that will be used to draw this material
		method->SetShader(shader);

		// Find and reuse the skybox created in the previous tutorial
		method->SetTexture(0, mGraphics->GetTexture("Skybox"));

		// The rest of this function hasn't changed
		mCam = FindObject<DebugCamera>(mCore->GetScene(), "Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update());
	}
}

//============================================================================================================
// The drawing process changes a fair bit. We will be going through the Render library now.
//============================================================================================================

void TestApp::OnDraw()
{
	mCore->BeginFrame();
	mCore->CullScene(mCam);

	// Deferred shading will need to know about all active lights in the scene (although we only have 1)
	const ILight::List& lights = mCore->GetScene()->GetVisibleLights();

	// Draw the scene using the deferred approach. This function (part of the Render library) does
	// a fair bit behind the scenes. In addition to all the setup and the actual deferred rendering
	// process it saves the final result into textures you can use for post-processing effects. This
	// is why the return type is not the 'uint' you've seen in the previous tutorials, but a struct
	// that also contains the color, depth and the eye-space normal textures.

	Deferred::DrawResult result = Deferred::DrawScene(mGraphics, lights, bind(&Core::DrawScene, mCore));

	// After the deferred draw function is done the result is stored in 3 off-screen textures.
	// The off-screen render target is still active, and we are free to draw some more. This would
	// be a perfect place to add some lens flares, transparent objects and particles. For simplicity's
	// sake though, let's just draw the result of the deferred drawing operation to the screen.

	// As you may recall, specifying '0' for the render target means we want to draw to the screen.
	// This also deactivates our off-screen buffers, allowing us to use the textures created above.
	mGraphics->SetActiveRenderTarget(0);

	// Activate the orthographic projection, deactivating the camera and preparing to draw in 2D.
	mGraphics->SetActiveProjection(IGraphics::Projection::Orthographic);

	// Just for the fun of it, let's add a bloom effect as we draw to the screen.
	// You might remember that in the previous tutorial we've loaded an HDR skybox with brightness
	// exceeding 1. Let's take advantage of that here, blooming values above 1. This will add a nice
	// glow effect to all bright areas as it draws our texture to the screen. Feel free to explore
	// the PostProcess namespace for additional effects.

	PostProcess::Bloom(mGraphics, result.mColor, 1.0f);

	// That's it! The rest of the application hasn't changed.
	mCore->EndFrame();
	Thread::Sleep(1);
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