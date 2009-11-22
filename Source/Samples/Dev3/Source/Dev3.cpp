#include "../Include/Dev3.h"
using namespace R5;

//============================================================================================================
// Constructor and destructor
//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mScene(0), mCam(0), mDeferred(true),
	mTriangles(0), mVisible(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI);
	mScene		= mCore->GetScene();
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
// Create a whole bunch of lights
//============================================================================================================

void TestApp::Init()
{
	Random rnd (625462456);

	mSet[0] = AddObject<Object>(mScene, "Lights 0");
	mSet[1] = AddObject<Object>(mScene, "Lights 1");
	mSet[2] = AddObject<Object>(mScene, "Lights 2");
	mSet[3] = AddObject<Object>(mScene, "Lights 3");

	// The sets are generated here so they shouldn't be saved out
	for (uint i = 0; i < 4; ++i) mSet[i]->SetSerializable(false);

	const ITexture* glowTex = mGraphics->GetTexture("light.jpg");
	//const ITexture* glareTex = mGraphics->GetTexture("glare_rgb.jpg");

	for (uint i = 0; i < 64; ++i)
	{
		PointLight* light = AddObject<PointLight>(mSet[i%4], String("Light %u", i));

		if (light != 0)
		{
			Glow* glow = AddObject<Glow>(light, String("Glow %u", i));
			glow->SetBackgroundTexture(glowTex);
			//glow->SetForegroundTexture(glareTex);

			Vector3f pos (rnd.GenerateFloat() * 70 - 35,
						  rnd.GenerateFloat() * 70 - 35,
						  rnd.GenerateFloat() * 4.0f  + 1.0f);
			Color3f clr (rnd.GenerateFloat(), rnd.GenerateFloat(), rnd.GenerateFloat());
			clr.Normalize();

			light->SetRange(10);
			light->SetBrightness(1.25f);
			light->SetPower(1.0f);
			light->SetAbsolutePosition(pos);
			light->SetDiffuse(clr);
			light->SetSpecular(clr);
		}
	}
}

//============================================================================================================
// Primary application loop
//============================================================================================================

void TestApp::Run()
{
    if (*mCore << "Config/Dev3.txt")
	{
		mCam = FindObject<Camera>(mCore->GetScene(), "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&TestApp::OnKey, this) );
			mCore->AddOnPostUpdate( bind(&TestApp::UpdateStats, this) );

			Init();

			while (mCore->Update());
		}
		//*mCore >> "Config/Dev3.txt";
	}
}

//============================================================================================================
// Draw callback
//============================================================================================================

void TestApp::OnDraw()
{
	mCore->BeginFrame();

	Animate();

	mCore->CullScene(mCam);

	mTriangles	= DrawScene();
	mVisible	= mScene->GetVisibleObjects().GetSize();

	mCore->DrawUI();
	mCore->EndFrame();
	Thread::Sleep(0);
}

//============================================================================================================
// Animate the lights
//============================================================================================================

void TestApp::Animate()
{
	float factor = Time::GetTime();

	Vector3f axis0 (0.0f, 0.01f, 1.0f);
	Vector3f axis1 (0.01f, 0.0f, 1.0f);
	Vector3f axis2 (0.01f, -0.01f, 1.0f);
	Vector3f axis3 (-0.005f, 0.015f, 1.0f);

	float factor0 =  factor * 0.93f;
	float factor1 = -factor * 0.68f;
	float factor2 =  factor * 0.47f;
	float factor3 = -factor * 1.8f;

	axis0.Normalize();
	axis1.Normalize();
	axis2.Normalize();
	axis3.Normalize();

	Quaternion a (axis0, factor0);
	Quaternion b (axis1, factor1);
	Quaternion c (axis2, factor2);
	Quaternion d (axis2, factor3);

	mSet[0]->SetRelativeRotation(a);
	mSet[1]->SetRelativeRotation(b);
	mSet[2]->SetRelativeRotation(c);
	mSet[3]->SetRelativeRotation(d);

	mSet[0]->SetRelativeScale( 0.75f * (1.0f + 0.5f * Float::Cos(factor0)) );
	mSet[1]->SetRelativeScale( 0.75f * (1.0f + 0.5f * Float::Cos(factor1)) );
	mSet[2]->SetRelativeScale( 0.75f * (1.0f + 0.5f * Float::Cos(factor2)) );
	mSet[3]->SetRelativeScale( 0.75f * (1.0f + 0.5f * Float::Cos(factor3)) );
}

//============================================================================================================
// Render the scene
//============================================================================================================

uint TestApp::DrawScene()
{
	static ITechnique*	opaque		= mGraphics->GetTechnique("Opaque");
	static ITechnique*	trans		= mGraphics->GetTechnique("Transparent");
	static ITechnique*	glow		= mGraphics->GetTechnique("Glow");
	static ITechnique*	glare		= mGraphics->GetTechnique("Glare");

	// Number of lights
	const ILight::List& lights = mScene->GetVisibleLights();
	uint lightCount = lights.GetSize();
	uint triangles = 0;

	if (mDeferred)
	{
		// Draw the scene using the deferred approach, filling color and depth buffers
		Deferred::DrawResult result = Deferred::DrawScene(mGraphics, lights, bind(&Core::DrawScene, mCore));
		triangles = result.mTriangles;

		// Add transparent objects via forward rendering
		{
			mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
			triangles += mCore->DrawScene(glow);
			triangles += mCore->DrawScene(glare);
		}

		// Draw the result onto the screen
		{
			mGraphics->SetActiveRenderTarget(0);
			mGraphics->SetActiveProjection( IGraphics::Projection::Orthographic );
			PostProcess::DepthOfField(mGraphics, result.mColor, result.mDepth, 20.0f, 7.0f, 14.0f);
		}

		mDebug.Set("Deferred (%u lights)", lightCount);
	}
	else
	{
		mGraphics->SetActiveRenderTarget(0);
		mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
		mGraphics->Clear();

		// Sort the lights so that closest lights are first
		mScene->GetVisibleLights( mCam->GetAbsolutePosition() );

		// Draw the scene
		triangles += mCore->DrawScene(opaque);
		triangles += mCore->DrawScene(trans);
		triangles += mCore->DrawScene(glow);
		triangles += mCore->DrawScene(glare);

		mDebug.Set("Forward (%u lights)", lightCount > 8 ? 8 : lightCount);
	}
	return triangles;
}

//============================================================================================================
// Registered update listener callback
//============================================================================================================

float TestApp::UpdateStats()
{
	static UILabel* tri = FindWidget<UILabel>(mUI, "Triangles");
	static UILabel* fps = FindWidget<UILabel>(mUI, "FPS");
	static UILabel* vis = FindWidget<UILabel>(mUI, "Visible");
	static UILabel* deb = FindWidget<UILabel>(mUI, "Debug");

	if (tri) tri->SetText( String("TRI: %u", mTriangles) );
	if (fps) fps->SetText( String("FPS: %u", Time::GetFPS()) );
	if (vis) vis->SetText( String("VIS: %u", mVisible) );
	if (deb) deb->SetText( mDebug );

	return 0.25f;
}

//============================================================================================================
// Triggered on keyboard and mouse button events
//============================================================================================================

bool TestApp::OnKey(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
		}
		else if ( key == Key::F5 )
		{
			mWin->SetStyle( mWin->GetStyle() == IWindow::Style::FullScreen ?
				IWindow::Style::Normal : IWindow::Style::FullScreen);
		}
		else if ( key == Key::F6 )
		{
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
		}
		else if ( key == Key::D )
		{
			mDeferred = !mDeferred;
		}
	}
	return true;
}

//============================================================================================================
// Application entry point
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