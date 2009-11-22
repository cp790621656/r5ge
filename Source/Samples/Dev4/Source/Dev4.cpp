#include "../Include/Dev4.h"
using namespace R5;

uint g_ssao = 1;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI);
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
    if (*mCore << "Config/Dev4.txt")
	{
		mCam = FindObject<Camera>(mCore->GetScene(), "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&TestApp::OnKey, this) );
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );

			while (mCore->Update());

			//*mCore >> "Config/Dev4.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	static Label* tri = FindWidget<Label>(mUI, "Triangles");
	static Label* fps = FindWidget<Label>(mUI, "FPS");
	static Label* inf = FindWidget<Label>(mUI, "Info");

	static ITechnique*	opaque	 = mGraphics->GetTechnique("Opaque");
	static ITechnique*	deferred = mGraphics->GetTechnique("Deferred");

	mCore->BeginFrame();
	mCore->CullScene(mCam);
	mCore->PrepareScene();

	uint triangles (0);

	if (g_ssao > 0)
	{
		const ILight::List& lights = mCore->GetScene()->GetVisibleLights();
		Deferred::DrawResult result = Deferred::DrawScene(mGraphics, lights, bind(&Core::DrawScene, mCore),
			((g_ssao % 2) == 0) ? SSAO::High : SSAO::Low);

		mGraphics->SetActiveRenderTarget(0);
		mGraphics->SetActiveProjection( IGraphics::Projection::Orthographic );
		PostProcess::None(mGraphics, result.mColor);
		triangles = result.mTriangles;
	}
	else
	{
		mGraphics->SetActiveRenderTarget(0);
		mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
		mGraphics->Clear();
	
		triangles += mCore->DrawScene(opaque);
	}

	if (tri) tri->SetText( String("TRI: %u", triangles) );
	if (fps) fps->SetText( String("FPS: %u", Time::GetFPS()) );
	if (inf) inf->SetText( String(g_ssao == 0 ? "No SSAO (S)" : (g_ssao == 1 ?
		"SSAO: Low Quality (S)" : "SSAO: High Quality (S)")) );

	mCore->DrawUI();
	mCore->EndFrame();
	Thread::Sleep(0);
}

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
		else if ( key == Key::S )
		{
			g_ssao += 1;
			if (g_ssao > 2) g_ssao = 0;
		}
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