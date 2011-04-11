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

	SpinScript() : mAxis(0.0f, 0.0f, 1.0f), mFactor(1.0f) {}

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
// Script attached to teapots via the template's OnSerialize functionality
//============================================================================================================

class TeapotScript : public Script
{
public:

	R5_DECLARE_INHERITED_CLASS("TeapotScript", TeapotScript, Script, Script);

	virtual void OnPreUpdate()
	{
		mObject->SetRelativeRotation( Quaternion(Vector3f(0.0f, 0.0f, 1.0f), -Time::GetTime()) );
	}
};

//============================================================================================================
// Constructor and destructor
//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	// Register a new script type that can be created via AddScript<> template
	Script::Register<SpinScript>();
	Script::Register<TeapotScript>();
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

	mCore->Lock();

	obj[0] = mCore->GetRoot()->AddObject<Object>("Lights 0");
	obj[0]->SetSerializable(false);
	obj[0]->AddScript<SpinScript>()->Set(0.0f, 0.01f, 1.0f, 0.93f);

	obj[1] = mCore->GetRoot()->AddObject<Object>("Lights 1");
	obj[1]->SetSerializable(false);
	obj[1]->AddScript<SpinScript>()->Set(0.01f, 0.0f, 1.0f, -0.68f);

	obj[2] = mCore->GetRoot()->AddObject<Object>("Lights 2");
	obj[2]->SetSerializable(false);
	obj[2]->AddScript<SpinScript>()->Set(0.01f, -0.01f, 1.0f, 0.47f);

	obj[3] = mCore->GetRoot()->AddObject<Object>("Lights 3");
	obj[3]->SetSerializable(false);
	obj[3]->AddScript<SpinScript>()->Set(-0.005f, 0.015f, 1.0f, -1.8f);

	const ITexture* glowTex = mGraphics->GetTexture("Billboards/light.jpg");
	//const ITexture* glareTex = mGraphics->GetTexture("Billboards/glare_rgb.jpg");

	const ITechnique* glow = mGraphics->GetTechnique("Glow");
	//const ITechnique* glare = mGraphics->GetTechnique("Glare");

	Random rnd (625462456);

	for (uint i = 0; i < 64; ++i)
	{
		PointLight* light = obj[i%4]->AddObject<PointLight>(String("Light %u", i));

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

			Billboard* bb = light->AddObject<Billboard>(String("Glow %u", i));
			bb->SetColor(clr);
			bb->SetTexture(glowTex);
			bb->SetTechnique(glow);

			//Glare* gl = AddObject<Glare>(light, String("Glare %u", i));
			//gl->SetTexture(glareTex);
			//gl->SetTechnique(glare);
			//gl->SetRelativeScale(4.0f);
		}
	}
	mCore->Unlock();
}

//============================================================================================================
// Primary application loop
//============================================================================================================

void TestApp::Run()
{
    if (*mCore << "Config/Dev3.txt")
	{
		Init();
		while (mCore->Update());
		//*mCore >> "Config/Dev3.txt";
	}
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