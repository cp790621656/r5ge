//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev9: Trees
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
	IWindow*	mWin;			// Application window
	IGraphics*	mGraphics;		// Graphics controller
	UI*			mUI;			// User interface manager
	Core*		mCore;			// Engine core
	Camera*		mCam;			// Main camera
	Scene		mScene;			// Main scene
	Scene		mOffscreen;		// Off-screen scene
	Mesh*		mBranch;		// Generated mesh used to draw the cluster of leaves
	uint		mSeed;			// Seed used for randomization
	Random		mRand;			// Random number generator

	ITexture*	mOriginalD;		// Original diffuse texture
	ITexture*	mOriginalN;		// Original normal map texture
	ITexture*	mFinalD;		// Generated diffuse texture for the cluster of leaves
	ITexture*	mFinalN;		// Generated normal map

	UISlider*	mSize;			// Slider controlling the size of individual leaves
	UISlider*	mTilt;			// Slider controlling the maximum tilting of the leaves
	UISlider*	mCount;			// Slider controlling the number of leaves in the generated mesh
	UIPicture*	mFinal;			// Widget with the final texture

public:

	TestApp();
	~TestApp();

	void Run();
	void OnDraw();
	void DrawLeaves(void* param);
	void Fill(Mesh::Vertices& verts, Mesh::Normals& normals, Mesh::TexCoords& tc, Mesh::Colors& colors);

	bool OnShowTexture(UIArea* area);
	bool OnStateChange(UIArea* area);
	bool OnGenerate(UIArea* area, const Vector2i& pos, byte key, bool isDown);
	void Generate();
	void OnSerializeTo (TreeNode& node) const;
	bool OnSerializeFrom (const TreeNode& node);
};

//============================================================================================================

TestApp::TestApp() : mBranch(0), mSeed(0), mOriginalD(0), mOriginalN(0), mFinalD(0), mFinalN(0),
	mSize(0), mTilt(0), mCount(0), mFinal(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI);

	Object* root = mCore->GetRoot();

	// Scene that contains regular visible objects
	mScene.SetRoot( AddObject<Object>(root, "Scene Root") );

	// Off-screen scene used for off-screen rendering
	mOffscreen.SetRoot( AddObject<Object>(root, "Off-screen Root") );
	mOffscreen.GetRoot()->SetSerializable(false);
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
// Bind listeners, load the configuration and enter the main loop
//============================================================================================================

void TestApp::Run()
{
	// Core event listeners
	mCore->SetListener( bind(&TestApp::OnSerializeTo,	this) );
	mCore->SetListener( bind(&TestApp::OnSerializeFrom, this) );
	mCore->SetListener( bind(&TestApp::OnDraw,			this) );

	// UI callbacks can be bound even before the actual widgets are loaded
	mUI->SetOnKey		 ("Generate",		bind(&TestApp::OnGenerate,		this));
	mUI->SetOnStateChange("Show",			bind(&TestApp::OnShowTexture,	this));
	mUI->SetOnStateChange("Normal Toggle",	bind(&TestApp::OnStateChange,	this));
	mUI->SetOnStateChange("Final Toggle",	bind(&TestApp::OnStateChange,	this));
	mUI->SetOnStateChange("Alpha Toggle",	bind(&TestApp::OnStateChange,	this));

	// Textures used by the application
	mOriginalD	= mGraphics->GetTexture("Leaf Diffuse map");
	mOriginalN	= mGraphics->GetTexture("Leaf Normal map");
	mFinalD		= mGraphics->GetTexture("Generated Diffuse map");
	mFinalN		= mGraphics->GetTexture("Generated Normal map");

	// Load the configuration
	if (*mCore << "Config/Dev9.txt")
	{
		// User Interface setup
		{
			// Widgets are defined in the configuration file
			mSize	= FindWidget<UISlider>(mUI, "Size");
			mTilt	= FindWidget<UISlider>(mUI, "Tilt");
			mCount	= FindWidget<UISlider>(mUI, "Count");
			mFinal	= FindWidget<UIPicture>(mUI, "Final");
		}

		// Camera setup
		{
			// Default camera's orientation comes from the configuration file
			mCam = FindObject<DebugCamera>(mScene, "Default Camera");

			if (mCam != 0)
			{
				mCore->SetListener( bind(&Camera::OnScroll, mCam) );
				mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			}

			// Off-screen camera is not serialized, but is rather created here
			DebugCamera* cam = AddObject<DebugCamera>(mOffscreen, "Off-screen Camera");
			cam->SetRelativeRotation( Vector3f(0.0f, 0.0f, -1.0f) );
			cam->SetRelativeRange( Vector3f(90.0f, 105.0f, 2.4f) );
			cam->SetDolly( Vector3f(0.0f, 100.0f, 100.0f) );
		}

		// Branch model setup
		{
			mBranch = mCore->GetMesh("Branch");
			Model* model = mCore->GetModel("Branch");

			// Set the mesh/material
			model->GetLimb("Branch")->Set(mBranch, mGraphics->GetMaterial("Leaf"));

			// Add this model to the off-screen scene
			ModelInstance* inst = AddObject<ModelInstance>(mOffscreen, "Branch");
			inst->SetModel(model);
			inst->SetSerializable(false);

			// Generate the initial mesh
			Generate();
		}

		// Enter the game loop
		while (mCore->Update()) {}

		//*mCore >> "Config/Dev9.txt";
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

void TestApp::OnDraw()
{
	mGraphics->SetActiveRenderTarget(0);
	mScene.Cull(mCam);
	mScene.DrawAllForward();
	mGraphics->Draw(IGraphics::Drawable::Grid);
}

//============================================================================================================
// Cull and draw the leaves into an off-screen buffer
//============================================================================================================

void TestApp::DrawLeaves(void* param)
{
	static DebugCamera* offCam = FindObject<DebugCamera>(mOffscreen, "Off-screen Camera");

	if (offCam != 0)
	{
		static IRenderTarget* diffuseTarget = 0;
		static IRenderTarget* normalTarget  = 0;
		ITexture* tex = mGraphics->GetTexture("Leaf Diffuse map");

		if (diffuseTarget == 0)
		{
			diffuseTarget = mGraphics->CreateRenderTarget();
			diffuseTarget->AttachColorTexture(0, mFinalD, ITexture::Format::RGBA);
			diffuseTarget->SetBackgroundColor( Color4ub(36, 56, 10, 0) );
			diffuseTarget->SetSize( tex->GetSize() * 2 );
		}

		if (normalTarget == 0)
		{
			normalTarget = mGraphics->CreateRenderTarget();
			normalTarget->AttachColorTexture(0, mFinalN, ITexture::Format::RGBA);
			normalTarget->SetBackgroundColor( Color4ub(127, 127, 255, 220) );
			normalTarget->SetSize( tex->GetSize() * 2 );
		}

		// Change the texture filtering to use 'nearest' filtering
		mFinalD->SetFiltering(ITexture::Filter::Nearest);
		mFinalN->SetFiltering (ITexture::Filter::Nearest);

		// Draw the scene into the diffuse map target
		mGraphics->SetActiveRenderTarget(diffuseTarget);
		mGraphics->Clear();
		mOffscreen.Cull(offCam);
		mOffscreen.Draw("Diffuse Map");

		// Draw the scene into the normal map target
		mGraphics->SetActiveRenderTarget(normalTarget);
		mGraphics->Clear();
		mOffscreen.Draw("Normal Map");

		// Turn alpha above 0 into a solid color -- we don't want the hideous alpha-bleeding
		// side-effect by-product of rendering alpha-blended textures into an FBO.
		mGraphics->SetFog(false);
		mGraphics->SetAlphaTest(false);
		mGraphics->SetBlending(IGraphics::Blending::None);
		mGraphics->SetActiveRenderTarget(diffuseTarget);
		mGraphics->SetActiveProjection( IGraphics::Projection::Orthographic );
		mGraphics->SetActiveTexture(0, mFinalD);
		mGraphics->SetActiveShader( mGraphics->GetShader("Other/RemoveAlpha") );
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );

		// Change filtering to anisotropic since the textures will be used on a 3D model
		mFinalD->SetFiltering(ITexture::Filter::Anisotropic);
		mFinalN->SetFiltering (ITexture::Filter::Anisotropic);
		mFinalD->SetWrapMode (ITexture::WrapMode::Repeat);
		mFinalN->SetWrapMode  (ITexture::WrapMode::Repeat);
	}
}

//============================================================================================================
// Fill the branch mesh geometry
//============================================================================================================

void TestApp::Fill (Mesh::Vertices& verts, Mesh::Normals& normals, Mesh::TexCoords& tc, Mesh::Colors& colors)
{
	mRand.SetSeed(mSeed);

	// How much the leaves will twist
	const float twist = HALFPI;

	// Color of the far-away vertices will be tinted by up to this amount (1.0 - tint)
	const float tint = 0.85f;
	const float minTint = 1.0f - tint;

	// Maximum color variance between the leaves
	const float variance = 0.25f;

	// Maximum depth range
	float depth = 5.0f;
	float invDepth = 1.0f / depth;

	// Maximum size of the particle
	float size = (mSize != 0 ? mSize->GetValue() * 0.5f : 0.2f);

	// Tilting bias -- higher means more tilting
	float bias = (mTilt != 0 ? mTilt->GetValue() : 0.5f);
	float invBias = 1.1f - bias;

	// Number of particles -- 1 to 500
	uint count = (mCount != 0 ? (uint)Float::Max(1.0f, mCount->GetValue() * 500.0f) : 100);

	// Maximum distance a particle can be spawned at without getting clipped
	float range = Float::Max(0.0f, 1.0f - size * 1.145f);

	Vector3f pos, axis, normal;
	Quaternion rot;
	float shade, rangeSq = range * range;

	// Create all leaf particles
	for (uint b = 0; b < count; ++b)
	{
		// Ensure that the generated particles are within circular bounds
		do 
		{
			pos.Set(mRand.GenerateRangeFloat() * range,
					mRand.GenerateRangeFloat() * range,
					0.0f);
		}
		while ((pos.x * pos.x + pos.y * pos.y) > range);		

		// Offset by depth
		pos.z = (depth * b) / count;

		axis.Set(mRand.GenerateRangeFloat() * bias, mRand.GenerateRangeFloat() * bias, invBias);
		axis.Normalize();

		rot.SetFromDirection( Vector3f(pos.x, pos.y, 0.0f) );
		rot *= Quaternion(axis, mRand.GenerateRangeFloat() * twist);

		// Top-left
		verts.Expand().Set(-size,  size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance, 1.0);

		// Bottom-left
		verts.Expand().Set(-size, -size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance, 1.0);

		// Bottom-right
		verts.Expand().Set( size, -size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance, 1.0);

		// Top-right
		verts.Expand().Set( size,  size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance,
								  shade + shade * mRand.GenerateRangeFloat() * variance, 1.0);

		// Calculate and set the normals
		normal.Set(0.0f, 0.0f, 1.0f);
		normal *= rot;

		normals.Expand() = normal;
		normals.Expand() = normal;
		normals.Expand() = normal;
		normals.Expand() = normal;

		// Texture coordinates are always the same
		tc.Expand().Set(0.0f, 1.0f);
		tc.Expand().Set(0.0f, 0.0f);
		tc.Expand().Set(1.0f, 0.0f);
		tc.Expand().Set(1.0f, 1.0f);
	}

	// Recreate the leaf texture at the beginning of the next frame
	mGraphics->ExecuteBeforeNextFrame( bind(&TestApp::DrawLeaves, this) );
}

//============================================================================================================
// Callback triggered when the state of N, A and F buttons changes
//============================================================================================================

bool TestApp::OnStateChange(UIArea* area)
{
	if (mFinal != 0)
	{
		UIWindow* parent = R5_CAST(UIWindow, area->GetParent());

		if (parent != 0)
		{
			UIButton* n = FindWidget<UIButton>(parent, "Normal Toggle", false);
			UIButton* f = FindWidget<UIButton>(parent, "Final Toggle", false);
			UIButton* a = FindWidget<UIButton>(parent, "Alpha Toggle", false);

			if (n != 0 && f != 0 && a != 0)
			{
				const char* title (0);

				if (f->GetState(UIButton::State::Pressed))
				{
					if (n->GetState(UIButton::State::Pressed))
					{
						title = "Generated Normal Map";
						mFinal->SetTexture(mFinalN);
						mFinal->IgnoreAlpha(true);
					}
					else
					{
						title = "Generated Diffuse Map";
						mFinal->SetTexture(mFinalD);
						mFinal->IgnoreAlpha(a->GetState(UIButton::State::Pressed));
					}
				}
				else
				{
					if (n->GetState(UIButton::State::Pressed))
					{
						title = "Original Normal Map";
						mFinal->SetTexture(mOriginalN);
						mFinal->IgnoreAlpha(true);
					}
					else
					{
						title = "Original Diffuse Map";
						mFinal->SetTexture(mOriginalD);
						mFinal->IgnoreAlpha(a->GetState(UIButton::State::Pressed));
					}
				}

				Vector2i size (mFinal->GetTexture()->GetSize());
				parent->ResizeToFit(size);
				parent->SetText( String("%s (%dx%d)", title, size.x, size.y) );
			}
		}
	}
	return true;
}

//============================================================================================================
// Callback triggered when the user clicks the "Show" button
//============================================================================================================

bool TestApp::OnShowTexture(UIArea* area)
{
	if (mFinal != 0)
	{
		UIButton* btn = R5_CAST(UIButton, area);

		if (btn != 0)
		{
			mFinal->GetParent()->SetAlpha( btn->GetState(UIButton::State::Pressed) ? 1.0f : 0.0f, 0.25f );
			OnStateChange(mFinal);
		}
	}
	return true;
}

//============================================================================================================
// Callback triggered when the user clicks on the "Generate" button
//============================================================================================================

bool TestApp::OnGenerate (UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft && !isDown)
	{
		mSeed = mRand.GenerateUint() + Time::GetMilliseconds();
		Generate();
	}
	return true;
}

//============================================================================================================
// Clear and regenerate the mesh used by the off-screen tree branch
//============================================================================================================

void TestApp::Generate()
{
	if (mBranch != 0)
	{
		mBranch->Lock();
		{
			mBranch->Clear();

			Fill(mBranch->GetVertexArray(),
				 mBranch->GetNormalArray(),
				 mBranch->GetTexCoordArray(),
				 mBranch->GetColorArray());

			mBranch->SetPrimitive(IGraphics::Primitive::Quad);
			mBranch->Update(true, false, true, true, false, false);
		}
		mBranch->Unlock();
	}
}

//============================================================================================================
// Save the seed used to generate the branch when serializing out
//============================================================================================================

void TestApp::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Seed", mSeed);
}

//============================================================================================================
// Load the previously saved seed used to generate the branch
//============================================================================================================

bool TestApp::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Seed")
	{
		node.mValue >> mSeed;
		return true;
	}
	return false;
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