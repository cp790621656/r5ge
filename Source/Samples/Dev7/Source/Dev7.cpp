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
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	mCore->SetSleepDelay(0);

	// Register the new fire and smoke emitters
	Object::Register<FireEmitter>();
	Object::Register<SmokeEmitter>();
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
		mCam = mScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			while (mCore->Update());

			//*mCore >> "Config/Dev7.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	static UILabel*	fps		= mUI->FindWidget<UILabel>("FPS");
	static UILabel*	tri		= mUI->FindWidget<UILabel>("Triangles");
	static Object*	place	= mScene.FindObject<Object>("Stage");

	mScene.Cull(mCam);

	mGraphics->Clear();
	mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
	mGraphics->Draw( IGraphics::Drawable::Grid );

	mScene.DrawAllForward(false);

	if (fps) fps->SetText( String("%u", Time::GetFPS()) );
	if (tri) tri->SetText( String("%u", mGraphics->GetFrameStats().mTriangles) );

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