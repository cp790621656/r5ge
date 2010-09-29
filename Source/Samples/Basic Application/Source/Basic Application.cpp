#include "../Include/Basic Application.h"
using namespace R5;

IGraphics* g_graphics = 0;

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
			const UIWidget& uiRoot = mWidget->GetUI()->GetRoot();
			TreeNode& tree = view->GetTree();
			uiRoot.SerializeTo(tree, false, true);
			view->SetDirty();
		}
	}
};

//============================================================================================================

struct USShowPosition : public UIScript
{
	UILabel* mLabel;
	USShowPosition() : mLabel(0) {}

	R5_DECLARE_INHERITED_CLASS("USShowPosition", USShowPosition, UIScript, UIScript);

	virtual void OnInit()
	{
		mLabel = R5_CAST(UILabel, mWidget);
		if (mLabel == 0) DestroySelf();
	}

	virtual void OnUpdate (bool changed)
	{
		if (mWidget->GetUI()->IsKeyDown(Key::T))
		{
			Vector3f pos (g_graphics->ConvertTo3D(mWidget->GetUI()->GetMousePos()));
			mLabel->SetText(String("%.2f %.2f %.2f", pos.x, pos.y, pos.z));
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
	UIScript::Register<USShowPosition>();

	g_graphics = mGraphics;
}

//============================================================================================================

TestApp::~TestApp()
{
	g_graphics = 0;

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