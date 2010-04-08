#include "../Include/Basic Application.h"
using namespace R5;

//============================================================================================================

struct USDestroyWindow : public UIScript
{
	R5_DECLARE_INHERITED_CLASS("USDestroyWindow", USDestroyWindow, UIScript, UIScript);

	virtual void OnKeyPress (const Vector2i& pos, byte key, bool isDown)
	{
		if (key == Key::MouseLeft && !isDown && mWidget->GetRegion().Contains(pos))
		{
			mWidget->GetParent()->DestroySelf();
		}
	}
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	UIScript::Register<USDestroyWindow>();
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
    if (*mCore << "Config/Basic Application.txt")
	{
		mCam = mScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			while (mCore->Update());

			//*mCore >> "Config/Basic Application.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllForward();
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