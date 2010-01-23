//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Tutorial 09: Better Terrain
//------------------------------------------------------------------------------------------------------------
// Ninth tutorial shows how to efficiently render a highly detailed terrain.
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
	void OnDraw();

	// New functions here: shader variables are bound to function callbacks. The terrain shader
	// we will be using in this tutorial will need to know the terrain's offset and scale values.
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
// A fair bit must be done in the Run() function...
//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/T09.txt")
	{
		// Find our terrain -- it's now loaded from the resource file
		Terrain* terrain = mScene.FindObject<Terrain>("First Terrain");

		if (terrain != 0)
		{
			// Remember the offset and scale values so we can pass them to the shader later
			mOffset.Set(-10.0f, -10.0f, -3.0f);
			mScale.Set(20.0f, 20.0f, 6.0f);

			// We will be generating a much higher quality terrain this time. Before we used 256x256, but
			// this time our height map will be 2048x2048 -- much more detailed! In addition we will be
			// generating a non-seamless height map this time, and it will only have one octave.

			Noise noise;
			noise.SetSize(2048, 2048);
			noise.SetSeamless(false);
			noise.ApplyFilter("Fractal");

			// Our desired mesh size remains at 32x32
			Terrain::Heightmap hm (noise.GetBuffer(), noise.GetWidth(), noise.GetHeight());
			hm.mMeshSize.Set(32, 32);
			hm.mTerrainOffset = mOffset;
			hm.mTerrainScale = mScale;

			// Here's a kicker. Although our height map is extremely highly detailed, let's only use one
			// partition to draw it. You might ask, "but won't that make our entire terrain only
			// 32 by 32 vertices?" The answer is yes. Yes it will.

			terrain->PartitionInto(1, 1);
			terrain->FillGeometry(&hm, 0.0f);

			// R5's terrain is extremely bare bone -- it only has vertices. No texture coordinates, no
			// normals, nothing else. The good news is that it doesn't need any of them. Texture coordinates
			// can be calculated from the positions of the vertices if the offset and scale used to generate
			// the terrain in the first place are provided. This is why we had to save them. As for the
			// normals? Well, we generated a 2048 by 2048 height map. Why don't we just convert that to a
			// normal map and simply stretch it over the terrain? This will effectively give us the illusion
			// of having an extremely detailed terrain without the performance cost of having all those
			// vertices to go along with it! Let's do that now.

			// Use the Image utility to generate a normal map
			Array<Color4ub> c;
			Image::HeightMapToNormalMap(noise.GetBuffer(), noise.GetWidth(), noise.GetHeight(), c, false, mScale);

			// Create a new texture called "Terrain"
			ITexture* tex = mGraphics->GetTexture("Terrain");

			// Turn this texture into our normal map by setting its pixels directly
			tex->Set(c, noise.GetWidth(), noise.GetHeight(), 1, ITexture::Format::RGBA);

			// Since our terrain is not seamless, it makes sense to set the texture to be clamped as well
			tex->SetWrapMode(ITexture::WrapMode::ClampToEdge);

			// Last thing we need to do is find the shader that will be used to draw our terrain...
			IShader* shader = mGraphics->GetShader("Forward/terrain");

			// ...and register our uniforms with it by specifying our callbacks. When the shader gets
			// activated our registered functions will be called, setting the variables in the shader.
			shader->RegisterUniform("g_offset", bind(&TestApp::SetOffset, this));
			shader->RegisterUniform("g_scale",  bind(&TestApp::SetScale,  this));

			// And that's it. If you look inside the resource file you will notice that the Terrain
			// material is now defined inside and that our terrain is using this material. The material
			// sets the shader to our "Terrain" shader, and sets the textures that will be used by
			// that shader. You might also notice that the light is no longer attached to the camera.
			// This was done so that it will be easier to see the shadowed parts of the terrain.
			// And now we have a 2048x2048 terrain rendered in only 2048 triangles.
		}

		// The rest of the Update function remains unchanged
		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Object::MouseMove, mCam) );
		mCore->SetListener( bind(&Object::Scroll, mCam) );

		while (mCore->Update());

		//*mCore >> "Config/T09.txt";
	}
}

//============================================================================================================
// The drawing function simply draws the entire scene with the default techniques
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllForward();
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