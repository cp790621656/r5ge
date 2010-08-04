#include "../Include/Dev4.h"
using namespace R5;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mDraw(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
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
		mCore->Lock();
		Object* obj = mCore->GetRoot()->FindObject<Object>("Default Camera");
		if (obj != 0) mDraw = obj->GetScript<OSDrawDeferred>();
		mCore->AddOnKey( bind(&TestApp::OnKeyPress, this) );
		mCore->Unlock();

		while (mCore->Update());
		//*mCore >> "Config/Dev4.txt";
	}
}

//============================================================================================================

uint TestApp::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
			return 1;
		}
		else if ( key == Key::F5 )
		{
			mWin->SetStyle( mWin->GetStyle() == IWindow::Style::FullScreen ?
				IWindow::Style::Normal : IWindow::Style::FullScreen);
			return 1;
		}
		else if ( key == Key::F6 )
		{
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
			return 1;
		}
		else if ( key == Key::S )
		{
			if (mDraw != 0)
			{
				byte ssao = mDraw->GetAOQuality() + 1;
				if (ssao > 2) ssao = 0;
				mDraw->SetAOQuality(ssao);
			}
			return 1;
		}
	}
	return 0;
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