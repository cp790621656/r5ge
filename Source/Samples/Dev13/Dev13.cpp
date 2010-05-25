//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev13: Shadows: WORK IN PROGRESS
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Render
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"

using namespace R5;

//============================================================================================================

class TestApp : Thread::Lockable
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mCamScene;
	Scene			mLightScene;
	Camera*			mCam;

public:

	TestApp();
	~TestApp();
	void Run();
	float OnDraw();
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mCamScene);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
    if (*mCore << "Config/Dev13.txt")
	{
		mCam = mCamScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			//// Create the second render target
			//// NOTE: If rendering to a deferred target, 'depth' is not required
			IRenderTarget* rt = mGraphics->CreateRenderTarget();

			// Secondary render target will always be 300x200
			rt->AttachColorTexture(0, mGraphics->GetTexture("Test"));
			rt->AttachDepthTexture(mGraphics->CreateRenderTexture());
			rt->SetSize( Vector2i(512, 512) );

			// Light scene will now be rendered into this render target
			mLightScene.SetRoot( mCamScene.GetRoot() );
			mLightScene.SetRenderTarget(rt);

			// Set the listener callbacks
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			// Draw callback
			mCore->AddOnDraw( bind(&TestApp::OnDraw, this) );

			// Update and draw the scene
			while (mCore->Update()) {}

			//*mCore >> "Config/Dev13.txt";
		}
	}
}

//============================================================================================================

Matrix44 g_shadowMat;

void SetShadowMatrix (const String& name, Uniform& data) { data = g_shadowMat; }

float TestApp::OnDraw()
{
	// Cull the scene from the camera's perspective
	mCamScene.Cull(mCam);

	// Inverse modelview matrix for the camera
	//Matrix43 imv (mCam->GetAbsolutePosition(), mCam->GetAbsoluteRotation(), mCam->GetAbsoluteScale());
	//imv.Invert();

	// Get the scene's calculated bounds and the light's inverse rotation
	// TODO: I need something like GetVisibleBounds() at scene level
	Bounds bounds = mCamScene.GetRoot()->GetCompleteBounds();
	static Object* light = mCamScene.GetRoot()->FindObject<Object>("First Light");
	light->SetAbsolutePosition(Vector3f());
	light->SetAbsoluteRotation(mCam->GetAbsoluteRotation());

	// Light's current rotation
	Quaternion rot (light->GetAbsoluteRotation());

	Vector3f extents ((bounds.GetMax() - bounds.GetMin()) * 0.5f);
	Vector3f center (bounds.GetCenter());

	// Reset the bounds to be based at the center of the original bounds
	bounds.Reset();
	bounds.Include(center + extents);
	bounds.Include(center - extents);

	// Transform the scene's bounds into light space
	bounds.Transform(Vector3f(), -rot, 1.0f);

	// Transformed size of the scene
	Vector3f size (bounds.GetMax() - bounds.GetMin());

	// Projection matrix should be an ortho box large enough to hold the entire transformed scene
	Matrix44 proj;
	proj.SetToBox(size.x, size.z, size.y);

	// Cull the light's scene
	mLightScene.Cull(center, rot, proj);

	// Bias matrix transforming -1 to 1 range into 0 to 1
	//static Matrix43 bias (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);

	//// Inverse camera view * light's modelview * bias
	//g_shadowMat = imv;
	//g_shadowMat *= mGraphics->GetModelViewProjMatrix();
	//g_shadowMat *= bias;

	//// Activate 
	//static IShader* shader = mGraphics->GetShader("Forward/shadowed_material");
	//if (shader != 0) shader->RegisterUniform("R5_shadowMatrix", SetShadowMatrix);

	// Draw the scene from the light's point of view
	//mLightScene._Draw("Depth");
	mLightScene.DrawAllForward();

	// TODO: Fix the need for this!
	//mCamScene.Cull(mCam);
	//mCamScene.DrawAllForward();
	mGraphics->SetActiveRenderTarget(0);
	mGraphics->Clear();

	// NOTES: For some reason the view matrix from the camera's scene gets inherited into the light's scene.
	// This shouldn't be happening, since I am manually forcing the view matrix using mLightScene.Cull().
	// This means that something is overwriting my value, and it's up to me to figure out what it is.
	return 0.0f;
}

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