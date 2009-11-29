//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Dev8: Projected Textures
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Render/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================

class SlightRotation : public Script
{
private:

	Quaternion	mOriginal;

public:

	R5_DECLARE_INHERITED_CLASS("Slight Rotation", SlightRotation, Script, Script);

	virtual void Init() { mOriginal = mObject->GetRelativeRotation(); }

	virtual void OnPreUpdate()
	{
		Quaternion rot = mOriginal;
		float time = Time::GetTime();
		Vector3f dir ( Float::Cos(time) * 3.0f, 5.0f, Float::Sin(time) * 2.0f );
		mObject->SetRelativeRotation(dir * rot);
	}
};

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mScene;
	DebugCamera*	mCam;
	Object*			mProj;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
	void DrawDecals(IGraphics* graphics, const ITexture* depth);

	void OnSetPos		(const String& name, Uniform& uni) { if (mProj != 0) uni = mProj->GetAbsolutePosition() * mGraphics->GetViewMatrix(); }
	void OnSetScale		(const String& name, Uniform& uni) { if (mProj != 0) uni = mProj->GetAbsoluteScale() * 0.5f; }
	void OnSetForward	(const String& name, Uniform& uni) { if (mProj != 0) uni = mProj->GetAbsoluteRotation().GetForward() % mGraphics->GetViewMatrix(); }
	void OnSetRight		(const String& name, Uniform& uni) { if (mProj != 0) uni = mProj->GetAbsoluteRotation().GetRight() % mGraphics->GetViewMatrix(); }
	void OnSetUp		(const String& name, Uniform& uni) { if (mProj != 0) uni = mProj->GetAbsoluteRotation().GetUp() % mGraphics->GetViewMatrix(); }
};

//============================================================================================================

TestApp::TestApp() : mCam(0), mProj(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	RegisterScript<SlightRotation>();
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
	if (*mCore << "Config/Dev8.txt")
	{
		mProj = FindObject<Object>(mScene, "Projected Box");

		IShader* shader = mGraphics->GetShader("Deferred/decal");
		shader->RegisterUniform("g_pos",	 bind(&TestApp::OnSetPos, this));
		shader->RegisterUniform("g_scale",	 bind(&TestApp::OnSetScale, this));
		shader->RegisterUniform("g_forward", bind(&TestApp::OnSetForward, this));
		shader->RegisterUniform("g_right",	 bind(&TestApp::OnSetRight, this));
		shader->RegisterUniform("g_up",		 bind(&TestApp::OnSetUp, this));

		mCam = FindObject<DebugCamera>(mScene, "Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update()) {}

		//*mCore >> "Config/Dev8.txt";
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	const Scene::Lights& lights = mScene.GetVisibleLights();
	Deferred::DrawResult result = Deferred::DrawScene(mGraphics, lights,
		bind(&Scene::Draw, &mScene), SSAO::High,
		bind(&TestApp::DrawDecals, this));
	PostProcess::None(mGraphics, result.mColor);

	static UILabel* fps = FindWidget<UILabel>(mUI, "FPS");
	if (fps) fps->SetText( String("FPS: %u", Time::GetFPS()) );
}

//============================================================================================================

void TestApp::DrawDecals(IGraphics* graphics, const ITexture* depth)
{
	static ITechnique* decal = graphics->GetTechnique("Decal");
	mProj->Draw(decal);
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