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
		IMaterial::RenderingMethod* method = mat->GetRenderingMethod(deferred);

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
// The drawing process changes a fair bit. We will be going through the Render library now.
//============================================================================================================

void TestApp::OnDraw()
{
	// Cull the scene like before, building the lists of visible objects and lights
	mScene.Cull(mCam);

	// Deferred shading will need to know about all active lights in the scene (although we only have 1)
	const Scene::Lights& lights = mScene.GetVisibleLights();

	// Draw the scene using the deferred approach. This function (part of the Render library) does
	// a fair bit behind the scenes. In addition to all the setup and the actual deferred rendering
	// process it saves the final result into textures you can use for post-processing effects. This
	// is why the return type is not the 'uint' you've seen in the previous tutorials, but a struct
	// that also contains the color, depth and the eye-space normal textures.

	Deferred::DrawResult result = Deferred::DrawScene(mGraphics, lights, bind(&Scene::Draw, &mScene));

	// After the deferred draw function is done the result is stored in 3 off-screen textures.
	// This might need more explanation. Up to this point we haven't changed the default render target, so
	// it remained being the screen. Deferred rendering changes the target in order to draw into off-screen
	// textures. You can always set your own render target via IGraphics::SetActiveRenderTarget() function.
	// if you specify '0' all following calls will go straight to the screen. But in any case, since
	// the deferred drawing render target is still active after the function call above, this would be
	// the perfect place to add some lens flares, transparent objects and particles that should go on top
	// of solid, lit objects. For simplicity's sake though, let's just draw the result of the deferred
	// drawing operation to the screen.

	// We can use the PostProcess namespace functionality to do just that (PostProcess::None), or
	// we can take advantage of the HDR skybox we've loaded in the previous tutorial with brightness
	// values exceeding 1 and actually add some HDR bloom. The call to do so is simple. Feel free to
	// explore the PostProcess namespace for additional effects or even add your own.

	PostProcess::Bloom(mGraphics, result.mColor, 1.0f);
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