//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 10: Object Scripts
//------------------------------------------------------------------------------------------------------------
// This tutorial shows how to use Object Scripts.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================
// Here we create a script that will be attached to our objects in the scene. In the resource file, when we
// define model instances we add an additional property: Script = "NameTag". When the resource file is
// loaded and the model instance is created, the NameTag script will be attached to the object, and will
// exist as long as the object exists (or until we choose to destroy it). Of course you can also add scripts
// programmatically without having to use the resource files. Just use the handy template:
//------------------------------------------------------------------------------------------------------------
// object->AddScript<NameTag>();
//============================================================================================================

class NameTag : public Script
{
	Core*		mCore;	// For convenience purposes we will remember the pointer to the Core
	UILabel*	mLabel;	// This will be our on-screen label

public:

	R5_DECLARE_INHERITED_CLASS("NameTag", NameTag, Script, Script);

	NameTag() : mCore(0), mLabel(0) {}

	// When the script is destroyed, we want to destroy the label it creates
	virtual ~NameTag()
	{
		if (mLabel != 0)
		{
			// Get the user interface from the core
			UI* ui = R5_CAST(UI, mCore->GetUI());

			// If the UI hasn't already been destroyed, delete the widget we created.
			// We do this because we don't want the on-screen label to stick around if
			// the script that was updating its position has been destroyed.
			if (ui != 0) mLabel->DestroySelf();
		}
	}

	// OnInit function is called when the script has been created and all internal parameters have been set
	virtual void OnInit()
	{
		// Every script belongs to an object, and all objects are aware of the engine's Core
		mCore = mObject->GetCore();

		// Get the user interface from the core
		UI* ui = R5_CAST(UI, mCore->GetUI());

		if (ui != 0)
		{
			// Add a label will be used to draw the object's position on-screen
			mLabel = ui->AddWidget<UILabel>(mObject->GetName());

			// We don't want this label to be serialized out as it was created dynamically
			mLabel->SetSerializable(false);

			// We also don't want it to receive events -- all clicks should fall right through it
			mLabel->SetEventHandling( UIWidget::EventHandling::None );

			// It should start off being invisible
			mLabel->SetAlpha(0.0f);
			mLabel->SetText(mObject->GetName());
			mLabel->SetAlignment(UILabel::Alignment::Center);
			mLabel->SetFont( ui->GetFont("Arial 15") );
		}

		// Although there is no way "mLabel" will be zero here, it's worth mentioning
		// that you can destroy the script in the OnInit() function -- or even the object
		// that owns the script. All DestroySelf calls are queued -- they are never executed
		// immediately, so it's perfectly safe to do anywhere except the constructor.
		if (mLabel == 0) DestroySelf();
	}

	// Called when the scene draw queue is being filled. This happens once per Scene::Cull call.
	virtual void OnFill (FillParams& params)
	{
		const Bounds&	bounds	= mObject->GetAbsoluteBounds();
		const Vector3f& center	= bounds.GetCenter();
		const Vector3f& max		= bounds.GetMax();

		// Point right above the top of the object
		Vector3f pos3 (center.x, center.y, max.z);

		// We only want to draw the label if the 3D point is actually visible to begin with
		if (params.mFrustum.IsVisible(pos3))
		{
			// Convert the 3D position to on-screen coordinates
			Vector2i pos2 (mCore->GetGraphics()->ConvertTo2D(pos3));

			// Update the label's region to be centered at that position
			mLabel->SetRegion(pos2.x - 50.0f, pos2.y - 10.0f, 100.0f, 20.0f);
			mLabel->SetAlpha(1.0f);
		}
		else
		{
			// If the point is not visible, there is really no point in displaying the label either
			mLabel->SetAlpha(0.0f);
		}
	}
};

//============================================================================================================
// Main class for our tutorial
//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	Core*			mCore;
	UI*				mUI;
	DebugCamera*	mCam;
	Scene			mScene;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp() : mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	// This is important: In order for our script to be recognized it
	// must first be registered. Let's do that now.
	Script::Register<NameTag>();
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
// In the Run function we only want to load the configuration files and enter the update loop
//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/T10.txt")
	{
		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Object::MouseMove, mCam) );
		mCore->SetListener( bind(&Object::Scroll, mCam) );

		while (mCore->Update());

		//*mCore >> "Config/T10.txt";
	}
}

//============================================================================================================
// The OnDraw function is simple: cull the scene, draw it using forward rendering, and draw a grid
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllForward(true);
	mGraphics->Draw( IGraphics::Drawable::Grid );
}

//============================================================================================================
// Application entry point hasn't changed
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