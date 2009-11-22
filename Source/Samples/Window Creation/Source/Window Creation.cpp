#include "../../../Engine/SysWindow/Include/_All.h"
using namespace R5;

//=============================================================================================================

struct TestWindow : public IEventReceiver
{
	String		mName;
	IWindow*	mWin;

	TestWindow()
	{
		mWin = new SysWindow();
		mWin->SetEventHandler(this);
	}

	~TestWindow()
	{
		if (mWin) delete mWin;
	}

	bool Create (const String& name, short x, short y, short w, short h, uint style)
	{
		mName = name;
		return mWin->Create(name, x, y, w, h, style);
	}

	bool Update()
	{
		return mWin->Update();
	}
	
	void BeginFrame()
	{
		mWin->BeginFrame();
	}

	void EndFrame()
	{
		mWin->EndFrame();
	}

	virtual bool OnMouseMove (const Vector2i& pos, const Vector2i& delta)
	{
		mWin->SetTitle( String("[%s]: %3d %3d (%3d %3d)", mName.GetBuffer(), pos.x, pos.y, delta.x, delta.y) );
		return true;
	}

	virtual bool OnKey (const Vector2i& pos, byte key, bool isDown)
	{
		printf("Key: 0x%x (%s)\n", key, isDown ? "true" : "false");
	
		if (!isDown)
		{
			if (key == Key::Escape) mWin->Close();
			else if (key == Key::G)
			{
				Vector2i p (mWin->GetPosition()),
						 s (mWin->GetSize());
				printf("[%s]: %ux%u\n", mName.GetBuffer(), pos.x, pos.y);
				printf("   %ux%u (%dx%u)\n", p.x, p.y, s.x, s.y);
			}
		}
		return true;
	}
	
	virtual void OnResize (const Vector2i& size)
	{
		printf("[%s]: Resized: %ux%u\n", mName.GetBuffer(), size.x, size.y);
	}
};

//=============================================================================================================

R5_MAIN_FUNCTION
{
	TestWindow win0;
	TestWindow win1;

	win0.Create("First",  100, 100, 400, 300, IWindow::Style::Normal);
	win1.Create("Second", 200, 200, 400, 300, IWindow::Style::Normal);
	//win1.Create("Second", 0, 0, 1280, 800, IWindow::Style::FullScreen);

	for (;;)
	{
		uint count (0);

		if (win0.Update())
		{
			win0.BeginFrame();
			win0.EndFrame();
			++count;
		}

		if (win1.Update())
		{
			win1.BeginFrame();
			win1.EndFrame();
			++count;
		}

		if (count == 0) break;
		Time::Update();
		Thread::Sleep(1);
	}
	return 0;
}