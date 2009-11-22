#include "../Include/Dev6.h"
using namespace R5;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mInst0(0), mInst1(0), mCam(0), mStatus(0), mMode(0), mTech(0)
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
    if (*mCore << "Config/Dev6.txt")
	{
		mCam = FindObject<Camera>(mCore->GetScene(), "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&TestApp::OnKey, this) );
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );

			mInst0		= FindObject<ModelInstance>(mCore->GetScene(), "Instance 0");
			mInst1		= FindObject<ModelInstance>(mCore->GetScene(), "Instance 1");
			mStatus		= FindWidget<Label>(mUI, "Status");
			mMode		= FindWidget<Label>(mUI, "Technique");

			ToggleTechnique();
			PlayAnimation(GetModel0(), "Walk");
			PlayAnimation(GetModel1(), "Run");

			while (mCore->Update());

			//*mCore >> "Config/Dev6.txt";
		}
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

void TestApp::OnDraw()
{
	static Label* fps = FindWidget<Label>(mUI, "FPS");
	static Label* tri = FindWidget<Label>(mUI, "Triangles");

	mCore->BeginFrame();
	mCore->CullScene(mCam);
	mCore->PrepareScene();
	{
		mGraphics->Draw( IGraphics::Drawable::Grid );
		uint triangles = mCore->DrawScene(mTech);
		if (fps) fps->SetText( String("%u", Time::GetFPS()) );
		if (tri) tri->SetText( String("%u", triangles) );
	}
	mCore->DrawUI();
	mCore->EndFrame();
	Thread::Sleep(0);
}

//============================================================================================================
// Toggles the current technique from GPU to CPU and vice versa
//============================================================================================================

void TestApp::ToggleTechnique()
{
	if (mTech != 0 && mTech->GetName() == "GPU")
	{
		mTech = mGraphics->GetTechnique("CPU");
		if (mMode != 0) mMode->SetText("[FFFF55]Skinning on CPU");
	}
	else
	{
		mTech = mGraphics->GetTechnique("GPU");
		if (mMode != 0) mMode->SetText("[55FF55]Skinning on GPU");
	}
}

//============================================================================================================
// Plays the specified animation on the specified model
//============================================================================================================

void TestApp::PlayAnimation (Model* model, const String& name, float speed)
{
	if (model != 0)
	{
		Animation* anim = model->GetAnimation(name);

		if (anim != 0)
		{
			model->SetAnimationSpeed(speed);
			model->PlayAnimation(anim, bind(&TestApp::OnAnimationStatus, this) );
		}
	}
}

//============================================================================================================
// Stops the playing animation
//============================================================================================================

void TestApp::StopAnimation (Model* model)
{
	if (model != 0)
	{
		Animation* anim = model->GetAnimation("Idle: Fiddle");
		if (anim != 0) model->StopAnimation(anim, 0.5f, bind(&TestApp::OnAnimationStatus, this));
	}
}

//============================================================================================================
// Delegate triggered when some animation is coming to an end -- it's a manually set callback
//============================================================================================================

void TestApp::OnAnimationStatus (Model* model, const Animation* anim, float timeToEnd)
{
	if (mStatus != 0)
	{
		mStatus->SetText( String("(%.3f) Animation '%s' ends in %.3f seconds", Time::GetTime(),
			anim->GetName().GetBuffer(), timeToEnd) );
	}
}

//============================================================================================================
// Function that responds to key events
//============================================================================================================

bool TestApp::OnKey(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
		}
		else if ( key == Key::W )
		{
			if (mInst0 != 0)
			{
				mInst0->SetShowOutline( !mInst0->IsShowingOutline() );
			}
		}
		else if ( key == Key::T )
		{
			ToggleTechnique();
		}
		else if ( key == Key::One )
		{
			PlayAnimation( GetRandomModel(), ((Time::GetMilliseconds() & 0x1) == 0) ? "Idle: Fiddle" : "Idle: Yawn" );
		}
		else if ( key == Key::Two )
		{
			PlayAnimation( GetRandomModel(), ((Time::GetMilliseconds() & 0x1) == 0) ? "Idle: Look Left" : "Idle: Look Right" );
		}
		else if ( key == Key::S )
		{
			StopAnimation( GetModel0() );
			StopAnimation( GetModel1() );
		}
		else if ( key == Key::F5 )
		{
			mWin->SetStyle( (mWin->GetStyle() & IWindow::Style::FullScreen) == 0 ?
							IWindow::Style::FullScreen : IWindow::Style::Normal );
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