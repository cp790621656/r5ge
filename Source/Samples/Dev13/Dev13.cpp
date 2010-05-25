//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev13: Shadows: WORK IN PROGRESS
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Render
//============================================================================================================
// TODO: Object's OnMouseMove and similar functionality should be removed, replaced with scripts.
// TODO: Object::GetTransformedBounds(rot, onlyVisible), should recurse through children
// TODO: Remove the need for a second Cull() call on the scene
// TODO: SetShadowMatrix() should be a built-in function -- likely at Core level
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"

using namespace R5;

//============================================================================================================
// A matrix is needed in order to transform vertices into light's texture space
//============================================================================================================

Matrix44 g_shadowMat;

void SetShadowMatrix (const String& name, Uniform& data) { data = g_shadowMat; }

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
	Object*			mLight;

public:

	TestApp();
	~TestApp();
	void Run();
	float OnDraw();
	bool MouseMove (const Vector2i& pos, const Vector2i& delta);
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0), mLight(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mCamScene);

	IShader* shader = mGraphics->GetShader("Forward/Shadowed_Material");
	if (shader != 0) shader->RegisterUniform("R5_shadowMatrix", SetShadowMatrix);
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
		mLight = mCamScene.FindObject<Object>("First Light");

		if (mCam != 0 && mLight != 0)
		{
			// Create the light's render target
			IRenderTarget* rt = mGraphics->CreateRenderTarget();
			rt->AttachDepthTexture(mGraphics->GetTexture("Light Depth"));
			rt->SetSize( Vector2i(1024, 1024) );

			// Light scene will now be rendered into this render target
			mLightScene.SetRoot( mCamScene.GetRoot() );
			mLightScene.SetRenderTarget(rt);

			// Set the listener callbacks
			mCore->SetListener( bind(&TestApp::MouseMove, this) );
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

float TestApp::OnDraw()
{
	// Cull the scene from the camera's perspective
	mCamScene.Cull(mCam);

	// Save the inverse modelview matrix -- it's needed for the shadow matrix
	Matrix43 inverseCameraMV (mGraphics->GetInverseModelViewMatrix());

	// Get the scene's calculated bounds
	Bounds bounds = mCamScene.GetRoot()->GetCompleteBounds();

	// Light's current rotation
	Quaternion rot (mLight->GetAbsoluteRotation());

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

	// Create a matrix that will transform the coordinates from camera's
	// modelview space to light's texture space
	{
		// Bias matrix transforming -1 to 1 range into 0 to 1
		static Matrix43 bias (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);

		// Tweak the projection matrix in order to remove z-fighting
		proj.Translate(Vector3f(0.0f, 0.0f, -0.1f / size.y));

		// Inverse camera view * light's modelview * bias
		g_shadowMat  = inverseCameraMV;
		g_shadowMat *= mGraphics->GetModelViewMatrix();
		g_shadowMat *= proj;
		g_shadowMat *= bias;
	}

	// Draw the scene from the light's point of view
	mGraphics->Clear();
	mLightScene._Draw("Depth");

	// Draw the shadowed scene
	mCamScene.Cull(mCam);
	mGraphics->Clear();
	mCamScene._Draw("Shadowed");
	return 0.0f;
}

//============================================================================================================

bool TestApp::MouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mCore->IsKeyDown(Key::L))
	{
		Quaternion rot (mLight->GetRelativeRotation());
		rot *= Quaternion(0.25f * DEG2RAD(delta.y), 0.0f, 0.25f * DEG2RAD(delta.x));
		mLight->SetRelativeRotation(rot);
	}
	else
	{
		mCam->MouseMove(pos, delta);
	}
	return true;
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