#include "../Include/_All.h"
using namespace R5;

R5::Random randomGen;

//============================================================================================================
// Constructor and destructor pair
//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI);

	// Register the new fire and smoke emitters
	RegisterObject<FireEmitter> (mCore->GetScene());
	RegisterObject<SmokeEmitter>(mCore->GetScene());
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
    if (*mCore << "Config/Dev7.txt")
	{
		mCam = FindObject<Camera>(mCore->GetScene(), "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );

			while (mCore->Update());

			//*mCore >> "Config/Dev7.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	static Label*	fps		= FindWidget<Label>(mUI, "FPS");
	static Label*	tri		= FindWidget<Label>(mUI, "Triangles");
	static Object*	place	= FindObject<Object>(mCore->GetScene(), "Stage");

	mCore->BeginFrame();
	mCore->CullScene(mCam);
	mCore->PrepareScene();
	{
		mGraphics->Draw( IGraphics::Drawable::Grid );
		uint triangles = mCore->DrawScene();

		if (fps) fps->SetText( String("%u", Time::GetFPS()) );
		if (tri) tri->SetText( String("%u", triangles) );
	}
	mCore->DrawUI();
	mCore->EndFrame();
	Thread::Sleep(0);

	if (place != 0)
	{
		place->SetRelativeRotation( Quaternion(0.0f, 0.0f, Time::GetTime()) );
	}
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