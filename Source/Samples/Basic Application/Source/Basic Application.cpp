#include "../Include/Basic Application.h"
using namespace R5;

Core* g_core = 0;

//============================================================================================================

class UITest : public UIScript
{
public:

	R5_DECLARE_INHERITED_CLASS("UITest", UITest, UIScript, UIScript);

	virtual void OnStateChange (uint state, bool isSet)
	{
		if (state == UIButton::State::Pressed && isSet)
		{
			Object* parent	= g_core->GetRoot()->FindObject<Object>("Teapot");
			Object* child	= g_core->GetRoot()->FindObject<Object>("Teapot 2");

			if (child->GetParent() == parent)
			{
				child->SetParent(0);
			}
			else
			{
				child->SetParent(parent);
			}
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

	g_core = mCore;

	UIScript::Register<UITest>();
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

#include <windows.h>

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