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

void SetShadowMatrix (const String& name, Uniform& uniform) { uniform = g_shadowMat; }

//============================================================================================================
// Creates a shadow by comparing light's depth to camera's depth
//============================================================================================================

void CreateShadow (IGraphics* graphics, Deferred::Storage& storage, const ITexture* lightDepth, const ITexture* camDepth, const Matrix44& mat)
{
	static IShader* shader = 0;
	static const ITechnique* technique = graphics->GetTechnique("Post Process");

	if (shader == 0)
	{
		shader = graphics->GetShader("Other/Shadow");
		shader->RegisterUniform("R5_shadowMatrix", SetShadowMatrix);
	}

	g_shadowMat = mat;

	graphics->SetActiveRenderTarget(storage.mRenderTarget);
	graphics->SetScreenProjection(true);
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);
	graphics->SetActiveShader(shader);
	graphics->SetActiveTexture(0, camDepth);
	graphics->SetActiveTexture(1, lightDepth);
	graphics->Clear();
	graphics->Draw( IGraphics::Drawable::InvertedQuad );

	// Update the final color texture
	storage.mOutColor = (storage.mRenderTarget == 0) ? 0 : storage.mRenderTarget->GetColorTexture(0);
}

//============================================================================================================

class TestApp : Thread::Lockable
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mCamScene;
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
	Vector2i targetSize = mWin->GetSize();

	static IRenderTarget* camTarget = 0;
	static IRenderTarget* lightTarget = 0;
	
	static ITexture* camDepth = mGraphics->GetTexture("Camera Depth");
	static ITexture* lightDepth = mGraphics->GetTexture("Light Depth");

	if (camTarget == 0)
	{
		camTarget = mGraphics->CreateRenderTarget();
		camTarget->AttachDepthTexture(camDepth);
	}

	if (lightTarget == 0)
	{
		// Color comparison meant for shadows
		lightDepth->SetCompareMode(ITexture::CompareMode::Shadow);

		lightTarget = mGraphics->CreateRenderTarget();
		lightTarget->AttachDepthTexture(lightDepth);
		lightTarget->SetSize( Vector2i(1024, 1024) );
	}

	// Update the render target's size
	camTarget->SetSize(targetSize);

	// Cull the scene from the camera's perspective and draw it into the "Camera Depth" texture
	mCamScene.SetRenderTarget(camTarget);
	mCamScene.Cull(mCam);
	mGraphics->Clear();
	mCamScene._Draw("Depth");

	// Matrix that will transform camera space to light space
	Matrix44 shadowMat;

	// Calculate the transformation matrix
	{
		// Use the same root
		Object* root = mCamScene.GetRoot();

		// Save the inverse modelview-projection matrix -- it's needed for the shadow matrix
		Matrix44 inverseCameraMVP (mGraphics->GetInverseMVPMatrix());

		// Light's rotation
		Quaternion rot (mLight->GetAbsoluteRotation());

		// Get the scene's calculated bounds and extents
		Bounds bounds (root->GetCompleteBounds());
		Vector3f extents ((bounds.GetMax() - bounds.GetMin()) * 0.5f);
		Vector3f center (bounds.GetCenter());

		// Transform the scene's bounds into light space
		bounds.Reset();
		bounds.Include(center + extents);
		bounds.Include(center - extents);
		bounds.Transform(Vector3f(), -rot, 1.0f);

		// Transformed size of the scene
		Vector3f size (bounds.GetMax() - bounds.GetMin());

		// Projection matrix should be an ortho box large enough to hold the entire transformed scene
		Matrix44 proj;
		proj.SetToBox(size.x, size.z, size.y);

		// Cull the light's scene
		// NOTE: Object's IsVisible flag is set on Cull... but it's called multiple times. Issue?
		static Scene lightScene;
		lightScene.SetRoot(root);
		lightScene.SetRenderTarget(lightTarget);
		lightScene.Cull(center, rot, proj);

		// Create a matrix that will transform the coordinates from camera to light space
		// Bias matrix transforming -1 to 1 range into 0 to 1
		static Matrix43 mvpToScreen (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);
		static Matrix43 screenToMVP (Vector3f(-1.0f, -1.0f, -1.0f), 2.0f);

		// Tweak the projection matrix in order to remove z-fighting
		proj.Translate(Vector3f(0.0f, 0.0f, -0.1f / size.y));

		// Inverse camera view * light's modelview * mvpToScreen
		shadowMat  = screenToMVP;
		shadowMat *= inverseCameraMVP;
		shadowMat *= mGraphics->GetModelViewMatrix();
		shadowMat *= proj;
		shadowMat *= mvpToScreen;

		// Draw the scene from the light's point of view, creating the "Light Depth" texture
		mGraphics->Clear();
		lightScene._Draw("Depth");
	}

	bool soft = true;

	static IRenderTarget* shadowTarget = 0;
	static ITexture* hardShadow = (soft) ? mGraphics->CreateRenderTexture() : mGraphics->GetTexture("Shadowmap");

	// Create the hard shadow texture
	{
		if (shadowTarget == 0)
		{
			shadowTarget = mGraphics->CreateRenderTarget();
			shadowTarget->AttachColorTexture(0, hardShadow, ITexture::Format::Alpha);
			shadowTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// Update the render target's properties
		shadowTarget->SetSize(targetSize);

		// Create the shadow texture
		mCamScene.SetRenderTarget(shadowTarget);
		CreateShadow(mGraphics, mCamScene, lightDepth, camDepth, shadowMat);
	}

	if (soft)
	{
		static IRenderTarget* blurTarget = 0;

		// Create the soft shadow texture
		{
			if (blurTarget == 0)
			{
				blurTarget = mGraphics->CreateRenderTarget();
				blurTarget->AttachColorTexture(0, mGraphics->GetTexture("Shadowmap"), hardShadow->GetFormat());
				blurTarget->SetBackgroundColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
			}

			// Update the render target's properties
			blurTarget->SetSize(targetSize);

			// Blur the shadow
			mCamScene.SetRenderTarget(blurTarget);
			PostProcess::Blur(mGraphics, mCamScene);
		}
	}

	// Restore the scene's render target
	mCamScene.SetRenderTarget(0);

	// TODO: Shouldn't need a second cull
	mCamScene.Cull(mCam);

	// Draw the scene using the generated shadow map
	mGraphics->Clear();
	mCamScene._Draw("Shadowed Opaque");
	return 0.0f;
}

//============================================================================================================

bool TestApp::MouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mCore->IsKeyDown(Key::L))
	{
		Quaternion relativeRot (mLight->GetRelativeRotation());

		// Horizontal
		{
			Quaternion rotQuat ( Vector3f(0.0f, 0.0f, 1.0f), 0.25f * DEG2RAD(delta.x) );
			relativeRot = rotQuat * relativeRot;
			relativeRot.Normalize();
		}

		// Vertical
		{
			Quaternion rotQuat ( relativeRot.GetRight(), 0.25f * DEG2RAD(delta.y) );
			relativeRot = rotQuat * relativeRot;
			relativeRot.Normalize();
		}

		mLight->SetRelativeRotation(relativeRot);
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