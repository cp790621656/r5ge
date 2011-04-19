//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
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
	Vector3f		mScale;
	Vector3f		mOffset;

public:

	TestApp();
	~TestApp();

	void Run();
	void GenerateTerrain();

	IMaterial* GenerateTest(Noise& heightmap, Array<Color4ub>& normalMap, Array<Color4ub>& colorMap, uint size);
	IMaterial* GenerateMountains(Noise& heightmap, Array<Color4ub>& normalMap, Array<Color4ub>& colorMap, uint size);

	void OnDraw();
	void SetOffset(const String& name, Uniform& uni) { uni = mOffset; }
	void SetScale (const String& name, Uniform& uni) { uni = mScale;  }
};

//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
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
	if (*mCore << "Config/Dev2.txt")
	{
		GenerateTerrain();
		while (mCore->Update());
		//*mCore >> "Config/Dev2.txt";
	}
}

//============================================================================================================

void TestApp::GenerateTerrain()
{
	Terrain* terrain = mCore->GetRoot()->FindObject<Terrain>("First Terrain");
	if (terrain == 0) return;

	mScale.Set(40.0f, 40.0f, 12.0f);
	mOffset = mScale * -0.5f;

	uint finalSize = 512;

	Noise heightMap;
	Array<Color4ub> normalMap;
	Array<Color4ub> colorMap;

	// Generate the terrain heightMap
	IMaterial* mat = GenerateMountains(heightMap, normalMap, colorMap, finalSize);
	//IMaterial* mat = GenerateTest(heightMap, normalMap, colorMap, finalSize);

	// Create the height map
	Terrain::Heightmap hm (heightMap.GetBuffer(), finalSize, finalSize);
	hm.mMeshSize.Set(128, 128);
	hm.mTerrainOffset = mOffset;
	hm.mTerrainScale  = mScale;

	// Partition and fill the terrain
	terrain->PartitionInto(1, 1);
	terrain->FillGeometry(&hm, 0.0f);
	terrain->SetMaterial(mat);

	// Create the normal map texture
	ITexture* normalTex = mGraphics->GetTexture("Terrain Normal Map");
	normalTex->Set(normalMap, finalSize, finalSize, 1, ITexture::Format::RGBA);
	normalTex->SetWrapMode(ITexture::WrapMode::ClampToEdge);

	// Create the colorMap map texture
	ITexture* gradientTex = mGraphics->GetTexture("Terrain Gradient Map");
	gradientTex->Set(colorMap, finalSize, finalSize, 1, ITexture::Format::RGBA);
	gradientTex->SetWrapMode(ITexture::WrapMode::ClampToEdge);

	// Set the shader uniforms
	IShader* shader = mGraphics->GetShader("Forward/dev2terrain");
	shader->RegisterUniform("g_offset", bind(&TestApp::SetOffset, this));
	shader->RegisterUniform("g_scale",  bind(&TestApp::SetScale,  this));
}

//============================================================================================================
// Generate the terrain from mixed noise
//============================================================================================================

IMaterial* TestApp::GenerateTest (Noise& heightMap, Array<Color4ub>& normalMap, Array<Color4ub>& colorMap, uint finalSize)
{
	// Reserve the heightmap
	heightMap.SetSize(finalSize, finalSize);
	heightMap.SetSeamless(false);
	heightMap.ApplyFilter("Fractal");
	heightMap.ApplyFilter("Erode").Set(30, 0.5f, 0.0f);
	heightMap.ApplyFilter("Normalize");

	// Reserve the normal map and color buffers
	normalMap.ExpandTo(finalSize * finalSize);
	colorMap.ExpandTo(finalSize * finalSize);

	// Normalize the heightMap result
	Normalize(heightMap.GetBuffer(), finalSize * finalSize);

	float* buffer = heightMap.GetBuffer();

	for (uint i = finalSize * finalSize; i > 0; )
	{
		float val = buffer[--i];
		buffer[i] = Float::Round(val, 0.05f);
	}

	// Create the normal map
	Image::HeightMapToNormalMap(heightMap.GetBuffer(), finalSize, finalSize, normalMap, false, mScale);

	for (uint i = finalSize * finalSize; i > 0; )
	{
		Color4ub& nm = normalMap[--i];
		Color4ub& cm = colorMap[i];

		Vector3f normal (nm);
		normal.Normalize();
		float slope = pow(normal.z, 10);
		cm.Set(Float::ToRangeByte(slope), 255, 255, 255);
	}

	// Use the test material
	return mGraphics->GetMaterial("Test");
}

//============================================================================================================
// Generate the terrain from mixed noise
//============================================================================================================

IMaterial* TestApp::GenerateMountains (Noise& heightMap, Array<Color4ub>& normalMap, Array<Color4ub>& colorMap, uint finalSize)
{
	uint erodedSize	= finalSize / 4;
	uint valleySize	= finalSize / 16;

	// Reserve the heightmap
	heightMap.SetSize(finalSize, finalSize);
	heightMap.SetSeamless(false);

	// Reserve the normal map and color buffers
	normalMap.ExpandTo(finalSize * finalSize);
	colorMap.ExpandTo(finalSize * finalSize);

	// Original fractal noise
	Noise fractal;
	fractal.SetSize(finalSize, finalSize);
	fractal.SetSeamless(heightMap.IsSeamless());
	fractal.ApplyFilter("Fractal");

	// Full sediment deposition thermal eroded noise
	Noise eroded;
	eroded.SetSize(erodedSize, erodedSize);
	eroded.SetSeamless(heightMap.IsSeamless());

	// No-deposition thermal eroded noise
	Noise carved;
	carved.SetSize(erodedSize, erodedSize);
	carved.SetSeamless(heightMap.IsSeamless());

	// Low resolution thermal eroded noise
	Noise valley;
	valley.SetSize(valleySize, valleySize);
	valley.SetSeamless(heightMap.IsSeamless());

	// Copy the fractal terrain into the eroded noise, downsampling it in the process
	memcpy(eroded.GetBuffer(), fractal.GetBuffer(eroded.GetSize()),
		sizeof(float) * erodedSize * erodedSize);

	// Carved noise starts with the same buffer as the eroded noise
	memcpy(carved.GetBuffer(), eroded.GetBuffer(), sizeof(float) * erodedSize * erodedSize);

	// Downsample the noise further
	memcpy(valley.GetBuffer(), carved.GetBuffer(valley.GetSize()),
		sizeof(float) * valleySize * valleySize);

	// Apply thermal erosion with full sediment deposition
	eroded.ApplyFilter("Erode").Set(30, 0.5f, 1.0f);

	// Carve out the terrain using a different erosion filter
	carved.ApplyFilter("Erode").Set(30, 0.5f, 0.0f);
	carved.ApplyFilter("Normalize");
	carved.ApplyFilter("Multiply").Set(1.05f);

	// Just like the eroded noise, valley noise uses full sediment deposition
	valley.ApplyFilter("Erode").Set(30, 0.5f, 1.0f);

	// Mix the two together
	float* fractalData	= fractal.GetBuffer();
	float* erodedData	= eroded.GetBuffer();
	float* carvedData	= carved.GetBuffer();
	float* valleyData	= valley.GetBuffer();
	float* finalData	= heightMap.GetBuffer();
	const float invSize = 1.0f / (finalSize - 1);

	for (uint y = 0; y < finalSize; ++y)
	{
		uint  yw = y * finalSize;
		float fy = y * invSize;

		for (uint x = 0; x < finalSize; ++x)
		{
			uint index = yw + x;
			float fx = x * invSize;

			float fractalPoint = fractalData[index];

			// Hermite texture sampling of the 3 input noises
			float carvedPoint	= Interpolation::HermiteClamp(carvedData, erodedSize, erodedSize, fx, fy);
			float erodedPoint	= Interpolation::HermiteClamp(erodedData, erodedSize, erodedSize, fx, fy);
			float valleyPoint	= Interpolation::HermiteClamp(valleyData, valleySize, valleySize, fx, fy);

			// Mix the 3 noises using the pixel's original relative height
			float carvedFactor	= Float::Clamp(fractalPoint - 0.5f,	0.0f, 0.5f) / 0.5f;
			float valleyFactor	= Float::Clamp(fractalPoint,		0.0f, 0.4f) / 0.4f;
			float fullErosion	= Interpolation::Linear(erodedPoint, carvedPoint, carvedFactor);
			fullErosion			= Interpolation::Linear(valleyPoint, fullErosion, valleyFactor);

			if (fullErosion > fractalPoint)
			{
				// Eroded pixel is higher than the original pixel
				finalData[index] = fullErosion;
				float r = (fullErosion - fractalPoint) / 0.0025f;
				r = Float::Clamp(r, 0.0f, 1.0f);
				colorMap[index].Set(Float::ToRangeByte(r), 170, 255, 255);
			}
			else
			{
				// Eroded pixel is lower than the original pixel
				finalData[index] = fractalPoint;
				colorMap[index].Set(0, 170, 255, 255);
			}
		}
	}

	// Normalize the heightMap result
	Normalize(heightMap.GetBuffer(), finalSize * finalSize);

	// Create the normal map
	Image::HeightMapToNormalMap(heightMap.GetBuffer(), finalSize, finalSize, normalMap, false, mScale);

	// Use the mountain material
	return mGraphics->GetMaterial("Mountains");
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