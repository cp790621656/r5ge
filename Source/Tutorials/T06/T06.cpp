//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 06: Materials
//------------------------------------------------------------------------------------------------------------
// Sixth tutorial covers materials, techniques, shaders, and the skybox.
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

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow(8);
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
// Changes the material on the model created in the previous tutorial
//============================================================================================================

void TestApp::Run()
{
	// Load a configuration file similar to the one used in the previous tutorial.
	if (*mCore << "Config/T06.txt")
	{
		mCore->Lock();

		// Find the model (automatically loads it if able)
		Model* model = mCore->GetModel("Models/teapot.r5a");

		// Create a new material we'll be using
		IMaterial* mat = mGraphics->GetMaterial("Chromatic");

		// By default, all materials are created with alpha of 0. Let's change that.
		mat->SetDiffuse( Color4f(1.0f) );

		// Create a new texture for the skybox we'll be using
		ITexture* skybox = mGraphics->GetTexture("Skybox");

		// We want to load all 6 sides of the skybox -- this means the texture will be a cube map.
		// Note that the format of the textures for the skybox is "HDR". This is because the skybox
		// is actually in radiance HDR format with brightness exceeding 1. Unfortunately forward
		// rendering can't handle brightness above 1. The next tutorial will remedy this issue.

		skybox->Load("Textures/Skyboxes/uffizi_pz.hdr",
					 "Textures/Skyboxes/uffizi_nz.hdr",
					 "Textures/Skyboxes/uffizi_py.hdr",
					 "Textures/Skyboxes/uffizi_px.hdr",
					 "Textures/Skyboxes/uffizi_ny.hdr",
					 "Textures/Skyboxes/uffizi_nx.hdr");

		// Since we have a skybox now, let's use it to replace the boring black background
		mGraphics->SetActiveSkybox(skybox);

		// All objects in the scene are drawn using one or more rendering techniques. Fully opaque
		// objects would be using a technique that renders everything front-to-back with alpha blending
		// disabled. Transparent objects would be doing the opposite: back-to-front draw order, drawn after
		// all opaque objects and with alpha blending enabled.
		
		// Each model needs to have one or more techniques set in order for it to be visible in the world,
		// and it will only be visible if you are actually drawing the scene with that technique. You
		// specify which technique to draw with by passing it to the Core::DrawScene() function, called
		// inside OnDraw() below. If you look at that function you will notice that we are not specifying
		// any parameters in this tutorial: this means that R5 is free to decide what to do. In this case
		// R5 will automatically draw the entire scene with all known forward rendering techniques
		// (opaque, transparent, particle, glow, then glare).

		ITechnique* tech = mGraphics->GetTechnique("Opaque");

		// Load the surface shader (it contains both vertex and fragment shaders)
		IShader* shader = mGraphics->GetShader("Surface/Chromatic");

		// Think of the DrawMethod like this: when rendering this material with the specified
		// technique, what shader and textures should we use? DrawMethod allows us to specify them.
		IMaterial::DrawMethod* method = mat->GetDrawMethod(tech);

		// We want to replace whatever shader was there with our chromatic shader loaded above.
		method->mShader = shader;

		// We'll use the skybox texture with the shader. If you look inside the shader itself, you
		// will see that it uses only a single cubemap texture -- the skybox.
		method->SetTexture(0, skybox);

		// All models are made up of limbs (which are actually just mesh/material pairs). Here our
		// model only has one limb, but other models may have more than one limb with different meshes
		// and materials associated with each one. World of Warcraft player models would be made out of
		// several limbs, as an example: boots, lower body, upper body, gloves, head, shoulders and helm.

		ModelTemplate::Limbs& limbs = model->GetAllLimbs();

		FOREACH(i, limbs)
		{
			// Associate our newly created material with the limb, replacing the material.
			Limb* limb = limbs[0];
			limb->SetMaterial(mat);
		}

		mCore->Unlock();
		while (mCore->Update());

		//*mCore >> "Config/T06.txt";
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