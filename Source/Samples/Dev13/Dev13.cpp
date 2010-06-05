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
// TODO: SetShadowMatrix() should be a built-in function -- likely at Core level
// Theirs: 0.1103 0.2466 0.6431
// Mine:   0.1429 0.2857 0.5714
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

Vector2f g_shadowOffset;

void SetShadowOffset (const String& name, Uniform& uniform) { uniform = g_shadowOffset; }

//============================================================================================================

Vector4f g_depthRange;

void SetDepthRange (const String& name, Uniform& uniform) { uniform = g_depthRange; }

//============================================================================================================
// Creates a shadow by comparing light's depth to camera's depth
//============================================================================================================

void CreateShadow (IGraphics* graphics, Deferred::Storage& storage, const ITexture* lightDepth, const Matrix44& mat)
{
	static IShader* shader = 0;
	static const ITechnique* technique = graphics->GetTechnique("Post Process");

	if (shader == 0)
	{
		shader = graphics->GetShader("Other/Shadow");
		shader->RegisterUniform("shadowMatrix", SetShadowMatrix);
		shader->RegisterUniform("shadowOffset", SetShadowOffset);
	}

	g_shadowMat = mat;
	g_shadowOffset = lightDepth->GetSize();

	// 1-pixel 30 degree rotated kernel (0.866, 0.5) multiplied by 2
	g_shadowOffset.x = 1.732f	/ g_shadowOffset.x;
	g_shadowOffset.y = 1.0f		/ g_shadowOffset.y;

	graphics->SetActiveRenderTarget(storage.mRenderTarget);
	graphics->SetScreenProjection(true);
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);
	graphics->SetActiveShader(shader);
	graphics->SetActiveTexture(0, storage.mOutDepth);
	graphics->SetActiveTexture(1, lightDepth);
	graphics->Clear();
	graphics->Draw( IGraphics::Drawable::InvertedQuad );

	// Update the final color texture
	storage.mOutColor = (storage.mRenderTarget == 0) ? 0 : storage.mRenderTarget->GetColorTexture(0);
}

//============================================================================================================
// Blur the shadow, creating a soft outline
//============================================================================================================

void BlurShadow (IGraphics* graphics, Deferred::Storage& storage, float near, float far, uint passes = 1)
{
	static IRenderTarget* blurTarget0 = 0;
	static IRenderTarget* blurTarget1 = 0;

	static ITexture* blurTex0 = graphics->CreateRenderTexture();
	static ITexture* blurTex1 = graphics->GetTexture("Shadowmap");

	static IShader*	blurH = graphics->GetShader("Other/blurShadowH");
	static IShader* blurV = graphics->GetShader("Other/blurShadowV");

	if (blurTarget0 == 0)
	{
		blurTarget0 = graphics->CreateRenderTarget();
		blurTarget0->AttachColorTexture(0, blurTex0, storage.mOutColor->GetFormat());

		blurTarget1 = graphics->CreateRenderTarget();
		blurTarget1->AttachColorTexture(0, blurTex1, storage.mOutColor->GetFormat());

		blurH->RegisterUniform("depthRange", SetDepthRange);
		blurV->RegisterUniform("depthRange", SetDepthRange);
	}

	Vector2i targetSize (storage.mOutColor->GetSize());
	blurTarget0->SetSize(targetSize);
	blurTarget1->SetSize(targetSize);

	// Update the depth range
	g_depthRange.Set(near, far, near * far, far - near);

	static const ITechnique* technique = graphics->GetTechnique("Post Process");

	graphics->SetScreenProjection(true);
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);
	graphics->SetActiveTexture(1, storage.mOutDepth);

	for (uint i = 0; i < passes; ++i)
	{
		graphics->SetActiveRenderTarget(blurTarget0);
		graphics->SetActiveShader(blurH);
		graphics->SetActiveTexture(0, storage.mOutColor);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
		storage.mOutColor = blurTex0;

		graphics->SetActiveRenderTarget(blurTarget1);
		graphics->SetActiveShader(blurV);
		graphics->SetActiveTexture(0, storage.mOutColor);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
		storage.mOutColor = blurTex1;
	}
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
	// Render target's size
	const Vector2i targetSize = mWin->GetSize();

	// Light's texture width and height
	const short lightTexSize = 2048;

	// Whether to create soft shadows
	const bool soft = true;

	mCamScene.mTempTargets.ExpandTo (32, true);
	mCamScene.mTempTextures.ExpandTo(32, true);

	// Scene's depth from the light's point of view
	Scene::ITexturePtr& lightDepth = mCamScene.mTempTextures[18];

	// Matrix that will transform camera space to light space
	Matrix44 shadowMat;

	// Generate the 'lightDepth' texture and calculate the 'shadowMat' transformation matrix
	{
		Scene::IRenderTargetPtr&	camTarget	= mCamScene.mTempTargets[14];
		Scene::IRenderTargetPtr&	lightTarget = mCamScene.mTempTargets[15];
		Scene::ITexturePtr&			camDepth	= mCamScene.mTempTextures[17];

		if (camTarget == 0)
		{
			// Create the temporary textures
			camDepth	= mGraphics->CreateRenderTexture();
			lightDepth	= mGraphics->CreateRenderTexture();

			// Set up the camera render target
			camTarget = mGraphics->CreateRenderTarget();
			camTarget->AttachDepthTexture(camDepth);

			// Color comparison meant for shadows
			lightDepth->SetCompareMode(ITexture::CompareMode::Shadow);

			// Set up the light render target
			lightTarget = mGraphics->CreateRenderTarget();
			lightTarget->AttachDepthTexture(lightDepth);
		}

		// Update the render target's size
		camTarget->SetSize(targetSize);

		// Use that dimension for the texture
		lightTarget->SetSize( Vector2i(lightTexSize, lightTexSize) );

		// Cull the scene from the camera's perspective and draw it into the "Camera Depth" texture
		mCamScene.SetRenderTarget(camTarget);
		mCamScene.Cull(mCam);
		mCamScene.Draw("Depth");

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
			// NOTE: Object's IsVisible flag is set on Cull... but it's called multiple times.
			// TODO: Figure out what to do with the isVisible flag: (isVisible |= actuallyVisible)?
			static Scene lightScene;
			lightScene.SetRoot(root);
			lightScene.SetRenderTarget(lightTarget);
			lightScene.Cull(center, rot, proj);

			// Create a matrix that will transform the coordinates from camera to light space
			// Bias matrix transforming -1 to 1 range into 0 to 1
			static Matrix43 mvpToScreen (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);
			static Matrix43 screenToMVP (Vector3f(-1.0f, -1.0f, -1.0f), 2.0f);

			// Tweak the projection matrix in order to remove z-fighting
			proj.Translate(Vector3f(0.0f, 0.0f, -(200.0f / lightTexSize) / size.y));

			// Inverse camera view * light's modelview * mvpToScreen
			shadowMat  = screenToMVP;
			shadowMat *= inverseCameraMVP;
			shadowMat *= mGraphics->GetModelViewMatrix();
			shadowMat *= proj;
			shadowMat *= mvpToScreen;

			// Draw the scene from the light's point of view, creating the "Light Depth" texture
			lightScene.Draw("Depth");
		}
	}

	// TODO: Remove the dependency on having to rely on a texture called "Shadowmap". As a reminder,
	// this is currently needed because "Shadowmap" is referenced from material configuration files.
	// Unfortunately this isn't right, as each render target will have its own shadowmap.
	// NOTE: This won't be a problem once deferred rendering is used.

	static ITexture* hardShadow = (soft) ? mGraphics->CreateRenderTexture() : mGraphics->GetTexture("Shadowmap");

	// Create the hard shadow texture by comparing the projection of light's depth onto the camera's depth
	{
		static IRenderTarget* shadowTarget = 0;

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
		CreateShadow(mGraphics, mCamScene, lightDepth, shadowMat);
	}

	// Blur the shadow creating a soft outline
	if (soft)
	{
		const Vector3f& range = mCam->GetAbsoluteRange();
		BlurShadow(mGraphics, mCamScene, range.x, range.y);
	}

	// Draw the scene using the generated shadow map
	mCamScene.SetRenderTarget(0);
	mCamScene.ActivateMatrices();
	mCamScene.Draw("Shadowed Opaque");
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