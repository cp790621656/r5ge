#include "../Include/Dev6.h"
using namespace R5;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0), mTech(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	mCore->SetSleepDelay(0);
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
	mUI->SetOnStateChange("Technique", bind(&TestApp::OnTechnique, this));

    if (*mCore << "Config/Dev6.txt")
	{
		mCam = mScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			for (uint i = 0; i < 7; ++i) mDebug[i] = mUI->FindWidget<UILabel>(String("Debug %u", i));

			Model* model0 = mCore->GetModel("First Model");
			Model* model1 = mCore->GetModel("Second Model");

			PlayAnimation(model0, "Walk");
			PlayAnimation(model1, "Run");

			for (int y = -10; y < 11; ++y)
			{
				for (int x = -10; x < 11; ++x)
				{
					ModelInstance* inst = mScene.AddObject<ModelInstance>(String("Instance %dx%d", x, y));
					inst->SetModel( (((x + y) & 1) == 0) ? model0 : model1 );
					inst->SetRelativePosition( Vector3f(2.0f * x, 2.0f * y, 0.0f) );
					inst->SetRelativeRotation( Vector3f(0.0f, -1.0f, 0.0f) );
					inst->SetSerializable(false);
				}
			}

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
	static UILabel* fps = mUI->FindWidget<UILabel>("FPS");
	static UILabel* tri = mUI->FindWidget<UILabel>("Triangles");

	mScene.Cull(mCam);

	Array<const ITechnique*> techs;
	techs.Expand() = mTech;

	mScene.DrawAllForward();
	mScene.Draw(techs);

	mGraphics->Draw( IGraphics::Drawable::Grid );
}

//============================================================================================================
// Toggles the current technique from GPU to CPU and vice versa
//============================================================================================================

void TestApp::OnTechnique (UIWidget* widget, uint state, bool isSet)
{
	UICheckbox* chk = R5_CAST(UICheckbox, widget);

	if (chk != 0 && (state & UICheckbox::State::Checked) != 0)
	{
		if (isSet)
		{
			mTech = mGraphics->GetTechnique("GPU");
			chk->SetText(" Skinning on [55FF55]GPU");
		}
		else
		{
			mTech = mGraphics->GetTechnique("CPU");
			chk->SetText(" Skinning on [FFFF55]CPU");
		}
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
			model->PlayAnimation(anim);
		}
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