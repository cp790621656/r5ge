#include "../Include/Basic Application.h"
using namespace R5;

//============================================================================================================

struct USCloseButton : public UIScript
{
	R5_DECLARE_INHERITED_CLASS("USCloseButton", USCloseButton, UIScript, UIScript);

	virtual void OnKeyPress (const Vector2i& pos, byte key, bool isDown)
	{
		if (key == Key::MouseLeft && !isDown && mWidget->GetRegion().Contains(pos))
		{
			UIWidget* parent = mWidget->GetParent();
			parent->AddScript<USFadeOut>()->SetDuration(2.0f);
		}
	}
};

//============================================================================================================

struct USFillTree : public UIScript
{
	R5_DECLARE_INHERITED_CLASS("USFillTree", USFillTree, UIScript, UIScript);

	virtual void OnInit()
	{
		UITreeView* view = R5_CAST(UITreeView, mWidget);

		if (view != 0)
		{
			mWidget->GetUI()->GetRoot().SerializeTo(view->GetTree());
			view->SetDirty();
		}
	}
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	UIScript::Register<USCloseButton>();
	UIScript::Register<USFillTree>();
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
		while (mCore->Update());
		//*mCore >> "Config/Basic Application.txt";
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