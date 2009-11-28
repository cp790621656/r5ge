//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Tutorial 08: Noise and Terrain
//------------------------------------------------------------------------------------------------------------
// Eighth tutorial in the series shows how to generate a quad-tree partitioned terrain.
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Render, Noise
//============================================================================================================

#include "../../Engine/Render/Include/_All.h"
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
	Terrain*		mTerrain;
	ITechnique*		mWireframe;
	UILabel*		mLabel;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp() : mCam(0), mTerrain(0), mWireframe(0), mLabel(0)
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
	if (*mCore << "Config/T08.txt")
	{
		// First we need to create a heightmap we'll use to create the terrain. R5 has a fairly flexible
		// noise library with a variety of simple filters that we can use for just that purpose.

		Noise noise;

		// We want to generate a 256x256 heightmap
		noise.SetSize(256, 256);

		// You can combine a variety of filters to create the terrain's "final" look, but for the sake
		// of simplicity, let's only use one -- a fractal field. The numbers that follow are optional
		// parameters. In this case '2' means generate a 2-octave noise, and 0.65 means that the noise
		// with values above 0.65 will be mirrored, turning high peaks into volcano-like crevices.
		// This type of noise is also known as ridged multifractal due to the ridges it tends to produce.

		noise.ApplyFilter("Fractal", "2 0.65");

		// Now that we have our heightmap, we should create our terrain.
		mTerrain = AddObject<Terrain>(mScene, "First Terrain");

		// We want to partition our terrain into an 8 by 8 grid. This will create 64 subdivisions
		// that the terrain will use together with frustum culling to automatically discard portions
		// of the terrain that are not visible. We can actually see what percentage of the terrain
		// is being rendered by using the Terrain::GetVisibility() function after the scene has been
		// culled... but more on that later.

		mTerrain->PartitionInto(8, 8);

		// In order to fill the terrain's partitions with geometry we need to provide additional
		// information about the heightmap that will be used and how it will be used to begin with.
		// Terrain::Heightmap struct exists for just this purpose.

		// Provide the heightmap itself
		Terrain::Heightmap hm (noise.GetBuffer(), noise.GetSize());

		// We want each subdivided mesh to be 32 by 32 quads. As you might recall there are 64 subdivisions
		// in total, and now each of those 64 will contain (32 x 32) = 1024 quads, or 2048 triangles.
		// When the terrain is generated the provided heightmap will be sampled using bicubic filtering,
		// so you can make the mesh much more tessellated than the heightmap, if you wish.

		hm.mMeshSize.Set(32, 32);

		// By default the terrain will be generated with dimensions of (0, 0, 0) to (1, 1, 1). Of course
		// that's not what we want. Let's apply a different scaling property here, stretching the terrain
		// along the horizontal plane (and a little bit along the vertical as well).
		hm.mTerrainScale.Set(20.0f, 20.0f, 4.0f);

		// By default the terrain starts at (0, 0, 0). Let's somwhat-center it instead.
		hm.mTerrainOffset.Set(-10.0f, -10.0f, -3.0f);

		// Time to fill the actual geometry. One last important thing to note is the optional bounding
		// box padding parameter that QuadTree::Fill function accepts. This parameter is used to extrude
		// the height of the bounding box vertically in both directions so that child objects can
		// fit easier. Objects that "fit" into the bounding box of the terrain's subdivisioned
		// nodes will get culled faster, speeding up your game. I recommend setting this property to
		// the height of the tallest building or tree you expect to place on your map. In this
		// example we don't have any objects placed as children of the terrain, but it's worth
		// noting nonetheless.

		mTerrain->Fill(&hm, 0.0f);

		// And now... we need to be able to see the terrain we've just created.
		// The best way to visualize a terrain without any textures on it is to display it in wireframe.
		// You can easily do that in R5 by using a material that has a "Wireframe" technique as one of
		// its draw methods. As long as you won't forget to use that technique in the OnDraw function,
		// your wireframe object will show up in your scene. In this case it will be used for our terrain.

		// Wireframe is an R5-recognized technique so we don't need to set up any states.
		mWireframe = mGraphics->GetTechnique("Wireframe");

		// We'll be using a custom material to draw our terrain. Let's just give it the same name.
		IMaterial* mat = mGraphics->GetMaterial("Terrain");

		// Add this technique to the material
		mat->GetDrawMethod(mWireframe, true);

		// Tell the terrain to use this material
		mTerrain->SetMaterial(mat);

		// Last thing we should do is find the label I've added to the "T08.txt" configuration file.
		// The reason it's not created via code is to simplify this tutorial. If you're curious,
		// have a look at that resource file and see how it was created inside. Since the label is
		// part of the configuration file that we've loaded at the top of this function, it's already
		// in memory and all we have to do is find it using this handy template:

		mLabel = FindWidget<UILabel>(mUI, "Status");

		// The rest of the Update function should be quite familiar to you.
		mCam = FindObject<DebugCamera>(mScene, "Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update());
	}
}

//============================================================================================================
// The drawing function returns to forward rendering
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mGraphics->Clear();

	// Rather than drawing the scene with all default techniques, let's only use ours
	mScene.Draw(mWireframe);

	// Since we have an on-screen label to play with, let's show how much of the terrain is currently visible
	if (mLabel != 0) mLabel->SetText( String("%.0f%%", mTerrain->GetVisibility() * 100.0f) );
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