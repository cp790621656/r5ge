#include "../Include/Dev3.h"
using namespace R5;

//============================================================================================================
// Script that will rotate and scale the platforms
//============================================================================================================

class SpinScript : public Script
{
	Vector3f	mAxis;
	float		mFactor;

public:

	R5_DECLARE_INHERITED_CLASS("SpinScript", SpinScript, Script, Script);

	void Set (float x, float y, float z, float factor)
	{
		mAxis = Normalize( Vector3f(x, y, z) );
		mFactor = factor;
	}

	virtual void OnPreUpdate()
	{
		float factor = Time::GetTime() * mFactor;
		mObject->SetRelativeRotation( Quaternion(mAxis, factor) );
		mObject->SetRelativeScale( 0.75f * (1.0f + 0.5f * Float::Cos(factor)) );
	}
};

//============================================================================================================
// Constructor and destructor
//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0), mObjects(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	// Register a new script type that can be created via AddScript<> template
	RegisterScript<SpinScript>();
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
// Create a whole bunch of lights
//============================================================================================================

void TestApp::Init()
{
	Object* obj[4];

	obj[0] = AddObject<Object>(mScene, "Lights 0");
	obj[0]->SetSerializable(false);
	AddScript<SpinScript>(obj[0])->Set(0.0f, 0.01f, 1.0f, 0.93f);

	obj[1] = AddObject<Object>(mScene, "Lights 1");
	obj[1]->SetSerializable(false);
	AddScript<SpinScript>(obj[1])->Set(0.01f, 0.0f, 1.0f, -0.68f);

	obj[2] = AddObject<Object>(mScene, "Lights 2");
	obj[2]->SetSerializable(false);
	AddScript<SpinScript>(obj[2])->Set(0.01f, -0.01f, 1.0f, 0.47f);

	obj[3] = AddObject<Object>(mScene, "Lights 3");
	obj[3]->SetSerializable(false);
	AddScript<SpinScript>(obj[3])->Set(-0.005f, 0.015f, 1.0f, -1.8f);

	const ITexture* glowTex = mGraphics->GetTexture("light.jpg");
	const ITexture* glareTex = mGraphics->GetTexture("glare_rgb.jpg");

	const ITechnique* glow = mGraphics->GetTechnique("Glow");
	//const ITechnique* glare = mGraphics->GetTechnique("Glare");

	Random rnd (625462456);

	for (uint i = 0; i < 64; ++i)
	{
		PointLight* light = AddObject<PointLight>(obj[i%4], String("Light %u", i));

		if (light != 0)
		{
			Vector3f pos (rnd.GenerateFloat() * 70 - 35,
						  rnd.GenerateFloat() * 70 - 35,
						  rnd.GenerateFloat() * 4.0f  + 1.0f);
			Color3f clr (rnd.GenerateFloat(), rnd.GenerateFloat(), rnd.GenerateFloat());
			clr.Normalize();

			light->SetRange(10);
			light->SetBrightness(1.25f);
			light->SetPower(1.0f);
			light->SetAbsolutePosition(pos);
			light->SetDiffuse(clr);
			light->SetSpecular(clr);

			Billboard* bb = AddObject<Billboard>(light, String("Glow %u", i));
			bb->SetColor(clr);
			bb->SetTexture(glowTex);
			bb->SetTechnique(glow);

			//Glare* gl = AddObject<Glare>(light, String("Glare %u", i));
			//gl->SetTexture(glareTex);
			//gl->SetTechnique(glare);
			//gl->SetRelativeScale(4.0f);
		}
	}

	// Techniques we'll be using
	mDeferred.Expand() = mGraphics->GetTechnique("Deferred");
	mForward.Expand()  = glow;
	//mForward.Expand()  = glare;
}

//============================================================================================================
// Primary application loop
//============================================================================================================

void TestApp::Run()
{
    if (*mCore << "Config/Dev3.txt")
	{
		mCam = FindObject<Camera>(mScene, "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->AddOnPostUpdate( bind(&TestApp::UpdateStats, this) );

			Init();

			while (mCore->Update());
		}
		//*mCore >> "Config/Dev3.txt";
	}
}

//============================================================================================================
// Draw callback
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	const Light::List& lights = mScene.GetVisibleLights();

	// Draw the scene using the deferred approach, filling color and depth buffers
	Deferred::DrawResult result = Deferred::DrawScene(mGraphics, lights, mDeferred, bind(&Scene::Draw, &mScene));

	// Add transparent objects via forward rendering
	mObjects += mScene.Draw(mForward);

	// Draw the result onto the screen
	//PostProcess::DepthOfField(mGraphics, result.mColor, result.mDepth, 20.0f, 7.0f, 14.0f);
	PostProcess::None(mGraphics, result.mColor);

	// Save statistics
	mObjects = result.mObjects;
	mStats	 = mGraphics->GetFrameStats();
}

//============================================================================================================
// Registered update listener callback
//============================================================================================================

float TestApp::UpdateStats()
{
	static UILabel* fps = FindWidget<UILabel>(mUI, "FPS");
	static UILabel* tri = FindWidget<UILabel>(mUI, "Triangles");
	static UILabel* obj = FindWidget<UILabel>(mUI, "Objects");
	static UILabel* db0 = FindWidget<UILabel>(mUI, "Debug 0");
	static UILabel* db1 = FindWidget<UILabel>(mUI, "Debug 1");

	if (tri) tri->SetText( String("TRI: %u", mStats.mTriangles) );
	if (fps) fps->SetText( String("FPS: %u", Time::GetFPS()) );
	if (obj) obj->SetText( String("%u objects", mObjects) );
	if (db0) db0->SetText( String("DC[[FF5555]%u[FFFFFF]] BB[[FF5555]%u[FFFFFF]] TX[[FF5555]%u[FFFFFF]]",
		mStats.mDrawCalls, mStats.mBufferBinds, mStats.mTexSwitches) );
	if (db1) db1->SetText( String("SH[[FF5555]%u[FFFFFF]] TC[[FF5555]%u[FFFFFF]] LS[[FF5555]%u[FFFFFF]]",
		mStats.mShaderSwitches, mStats.mTechSwitches, mStats.mLightSwitches) );

	return 0.25f;
}

//============================================================================================================
// Application entry point
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