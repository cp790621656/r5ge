//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev2: Terrain
//------------------------------------------------------------------------------------------------------------
// Development testing environment for terrain.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Noise
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Noise/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mScene;
	DebugCamera*	mCam;
	Vector3f		mScale;
	Vector3f		mOffset;

public:

	TestApp();
	~TestApp();
	void Run();
	void GenerateTerrain();
	void OnDraw();
	void SetOffset(const String& name, Uniform& uni) { uni = mOffset; }
	void SetScale (const String& name, Uniform& uni) { uni = mScale;  }
};

//============================================================================================================

TestApp::TestApp() : mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);
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
	if (*mCore << "Config/Dev2.txt")
	{
		GenerateTerrain();

		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update());

		//*mCore >> "Config/Dev2.txt";
	}
}

//============================================================================================================

void TestApp::GenerateTerrain()
{
	Terrain* terrain = mScene.FindObject<Terrain>("First Terrain");
	if (terrain == 0) return;

	mOffset.Set(-20.0f, -20.0f, -6.0f);
	mScale.Set(40.0f, 40.0f, 12.0f);

	uint finalSize = 2048;

	Noise final;
	final.SetSize(finalSize, finalSize);
	final.SetSeamless(false);

	Array<Color4ub> gradientMap;
	gradientMap.ExpandTo(finalSize * finalSize);

	// Generate the terrain from mixed noise
	{
		uint erodedSize = finalSize / 4;

		Noise fractal;
		fractal.SetSize(finalSize, finalSize);
		fractal.SetSeamless(final.IsSeamless());
		fractal.ApplyFilter("Fractal");

		Noise eroded;
		eroded.SetSize(erodedSize, erodedSize);
		eroded.SetSeamless(final.IsSeamless());

		// Copy the fractal terrain into the eroded noise, downsampling it in the process
		memcpy(eroded.GetBuffer(), fractal.GetBuffer(eroded.GetSize()),
			sizeof(float) * erodedSize * erodedSize);

		// Apply erosion
		eroded.ApplyFilter("Erode", "30 0.5 0.0");
		eroded.ApplyFilter("Normalize");
		eroded.ApplyFilter("Multiply", 1.05f);
		eroded.ApplyFilter("Add", 0.015f);

		// Mix the two together
		float* fractalData	= fractal.GetBuffer();
		float* erodedData	= eroded.GetBuffer();
		float* finalData	= final.GetBuffer();
		const float invSize = 1.0f / (finalSize - 1);

		for (uint y = 0; y < finalSize; ++y)
		{
			uint  yw = y * finalSize;
			float fy = y * invSize;

			for (uint x = 0; x < finalSize; ++x)
			{
				uint index = yw + x;
				float fx = x * invSize;
				float erodedPoint = Interpolation::HermiteClamp(erodedData, erodedSize, erodedSize, fx, fy);
				float fractalPoint = fractalData[index];

				if (erodedPoint > fractalPoint)
				{
					finalData[index] = erodedPoint;
					float f = (erodedPoint - fractalPoint) / 0.005f;
					f = Float::Clamp(f, 0.0f, 1.0f);
					f = f * f * f;
					gradientMap[index].Set(Float::ToNormalMapByte(f), 0, 0, 0);
				}
				else
				{
					finalData[index] = fractalPoint;
					gradientMap[index].Set(0, 0, 0, 0);
				}
			}
		}
	}

	// Normalize the final result
	Normalize(final.GetBuffer(), finalSize * finalSize);

	// Create the normal map
	Array<Color4ub> normalMap;
	Image::HeightMapToNormalMap(final.GetBuffer(), finalSize, finalSize, normalMap, false, mScale);

	// Create the height map
	Terrain::Heightmap hm (final.GetBuffer(), finalSize, finalSize);
	hm.mMeshSize.Set(128, 128);
	hm.mTerrainOffset = mOffset;
	hm.mTerrainScale  = mScale;

	// Partition and fill the terrain
	terrain->PartitionInto(1, 1);
	terrain->FillGeometry(&hm, 0.0f);

	// Create the normal map texture
	ITexture* normalTex = mGraphics->GetTexture("Terrain Normal Map");
	normalTex->Set(normalMap, finalSize, finalSize, 1, ITexture::Format::RGBA);
	normalTex->SetWrapMode(ITexture::WrapMode::ClampToEdge);

	// Create the gradientMap map texture
	ITexture* gradientTex = mGraphics->GetTexture("Terrain Gradient Map");
	gradientTex->Set(gradientMap, finalSize, finalSize, 1, ITexture::Format::RGBA);
	gradientTex->SetWrapMode(ITexture::WrapMode::ClampToEdge);

	// Set the shader uniforms
	IShader* shader = mGraphics->GetShader("Forward/dev2terrain");
	shader->RegisterUniform("g_offset", bind(&TestApp::SetOffset, this));
	shader->RegisterUniform("g_scale",  bind(&TestApp::SetScale,  this));
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllForward();
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