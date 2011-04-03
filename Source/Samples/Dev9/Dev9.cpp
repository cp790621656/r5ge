//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
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
	Scene		mOffscreen;		// Off-screen scene
	Mesh*		mBranch;		// Generated mesh used to draw the cluster of leaves
	uint		mSeed;			// Seed used for randomization
	R5::Random	mRand;			// Random number generator

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
	void DrawLeaves (IGraphicsManager* graphics, void* param);
	void SaveTextures (IGraphicsManager* graphics, void* param);
	void Fill(Mesh::Vertices& verts, Mesh::Normals& normals, Mesh::TexCoords& tc, Mesh::Colors& colors);

	void OnShowTexture	(UIWidget* widget, uint state, bool isSet);
	void OnStateChange	(UIWidget* widget, uint state, bool isSet);
	void OnGenerate		(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	void OnSave			(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	void Generate();
};

//============================================================================================================

TestApp::TestApp() : mBranch(0), mSeed(2711135145), mOriginalD(0), mOriginalN(0), mFinalD(0), mFinalN(0),
	mSize(0), mTilt(0), mCount(0), mFinal(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	Object* root = mCore->GetRoot();

	// Off-screen scene used for off-screen rendering
	mOffscreen.SetRoot( root->AddObject<Object>("Off-screen Root") );
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
	// UI callbacks can be bound even before the actual widgets are loaded
	mUI->SetOnKey		 ("Save",			bind(&TestApp::OnSave,			this));
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
			mSize	= mUI->FindWidget<UISlider>("Size");
			mTilt	= mUI->FindWidget<UISlider>("Tilt");
			mCount	= mUI->FindWidget<UISlider>("Count");
			mFinal	= mUI->FindWidget<UIPicture>("Final");
		}

		// Camera setup
		{
			// Off-screen camera is not serialized, but is rather created here
			DebugCamera* cam = mOffscreen.AddObject<DebugCamera>("Off-screen Camera");
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
			ModelInstance* inst = mOffscreen.AddObject<ModelInstance>("Branch");
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
// Cull and draw the leaves into an off-screen buffer
//============================================================================================================

void TestApp::DrawLeaves (IGraphicsManager* graphics, void* param)
{
	static DebugCamera* offCam = mOffscreen.FindObject<DebugCamera>("Off-screen Camera");

	if (offCam != 0)
	{
		static IRenderTarget* diffuseTarget = 0;
		static IRenderTarget* normalTarget  = 0;
		static ITexture* temp0 = mGraphics->GetTexture("Temp Texture 0");
		static ITexture* temp1 = mGraphics->GetTexture("Temp Texture 1");

		ITexture* tex = mGraphics->GetTexture("Leaf Diffuse map");

		if (diffuseTarget == 0)
		{
			diffuseTarget = mGraphics->CreateRenderTarget();
			diffuseTarget->AttachColorTexture(0, temp0, ITexture::Format::RGBA);
			diffuseTarget->SetSize( tex->GetSize() * 2 );
			temp0->SetFiltering(ITexture::Filter::Nearest);
		}

		if (normalTarget == 0)
		{
			normalTarget = mGraphics->CreateRenderTarget();
			normalTarget->AttachColorTexture(0, temp1, ITexture::Format::RGBA);
			normalTarget->SetSize( tex->GetSize() * 2 );
			temp1->SetFiltering(ITexture::Filter::Nearest);
		}

		// Draw the scene into the diffuse map target
		{
			// Off-screen target
			mOffscreen.SetFinalTarget(diffuseTarget);
			mOffscreen.Cull(offCam);
			mGraphics->SetBackgroundColor( Color4ub(65, 90, 20, 0) );
			mOffscreen.DrawWithTechnique("Diffuse Map", true, true, true);
			mGraphics->Flush();

			// Turn alpha above 0 into a solid color -- we don't want the hideous alpha-bleeding
			// side-effect by-product of rendering alpha-blended textures into an FBO.
			mGraphics->SetFog(false);
			mGraphics->SetAlphaTest(false);
			mGraphics->SetBlending(IGraphics::Blending::None);
			mGraphics->SetActiveRenderTarget(diffuseTarget);
			mGraphics->SetScreenProjection( true );
			mGraphics->SetActiveTexture(0, temp0);
			mGraphics->SetActiveShader( mGraphics->GetShader("Other/RemoveAlpha") );
			mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
			mGraphics->Flush();
		}

		// Draw the scene into the normal map target
		{
			mOffscreen.SetFinalTarget(normalTarget);
			mOffscreen.ActivateMatrices();
			mGraphics->SetBackgroundColor( Color4ub(127, 127, 255, 220) );
			mOffscreen.DrawWithTechnique("Normal Map", true, true, true);
			mGraphics->Flush();
		}

		// Get the texture data and fill out the diffuse and normal textures
		{
			Memory mem;

			temp0->GetBuffer(mem);
			mFinalD->Set(mem.GetBuffer(), temp0->GetSize().x, temp0->GetSize().y, 1, temp0->GetFormat());
			mFinalD->SetFiltering(ITexture::Filter::Anisotropic);
			mFinalD->SetWrapMode(ITexture::WrapMode::Repeat);

			temp1->GetBuffer(mem);
			mFinalN->Set(mem.GetBuffer(), temp1->GetSize().x, temp1->GetSize().y, 1, temp1->GetFormat());
			mFinalN->SetFiltering(ITexture::Filter::Anisotropic);
			mFinalN->SetWrapMode(ITexture::WrapMode::Repeat);
		}
	}
}

//============================================================================================================
// Callback executed on the graphics thread, saves the leaf textures
//============================================================================================================

void TestApp::SaveTextures (IGraphicsManager* graphics, void* param)
{
	Memory diffuse, normal;

	const Vector2i& d = mFinalD->GetSize();
	const Vector2i& n = mFinalN->GetSize();

	static uint counter = 0;

	if (mFinalD->GetBuffer(diffuse))
	{
		String filename ("leaves%3u.tga", counter);
		filename.Replace(" ", "0");
		Image::StaticSave(filename, diffuse.GetBuffer(), d.x, d.y, mFinalD->GetFormat());
	}
	
	if (mFinalN->GetBuffer(normal))
	{
		String filename ("leaves%3u_nm.tga", counter);
		filename.Replace(" ", "0");
		Image::StaticSave(filename, normal.GetBuffer(), n.x, n.y, mFinalN->GetFormat());
	}
	++counter;
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
	float shade;

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
// Callback triggered when the user clicks the "Show" button
//============================================================================================================

void TestApp::OnShowTexture(UIWidget* widget, uint state, bool isSet)
{
	if (mFinal != 0 && (state & UIButton::State::Pressed) != 0)
	{
		mFinal->GetParent()->SetAlpha(isSet ? 1.0f : 0.0f, 0.25f );
		OnStateChange(mFinal, 0, false);
	}
}

//============================================================================================================
// Callback triggered when the state of N, A and F buttons changes
//============================================================================================================

void TestApp::OnStateChange(UIWidget* widget, uint state, bool isSet)
{
	if (mFinal != 0)
	{
		UIWindow* parent = R5_CAST(UIWindow, widget->GetParent());

		if (parent != 0)
		{
			UIButton* n = parent->FindWidget<UIButton>("Normal Toggle", false);
			UIButton* f = parent->FindWidget<UIButton>("Final Toggle", false);
			UIButton* a = parent->FindWidget<UIButton>("Alpha Toggle", false);

			if (n != 0 && f != 0 && a != 0)
			{
				const char* title (0);

				if (f->GetState(UIButton::State::Pressed))
				{
					if (n->GetState(UIButton::State::Pressed))
					{
						title = "Generated Normal Map";
						mFinal->SetTexture(mFinalN);
						mFinal->IgnoreAlpha(a->GetState(UIButton::State::Pressed));
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
						mFinal->IgnoreAlpha(a->GetState(UIButton::State::Pressed));
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
}

//============================================================================================================
// Triggered when the "Save" button gets clicked on
//============================================================================================================

void TestApp::OnSave (UIWidget* widget, const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft && !isDown)
	{
		mGraphics->ExecuteBeforeNextFrame( bind(&TestApp::SaveTextures, this) );
	}
}

//============================================================================================================
// Callback triggered when the user clicks on the "Generate" button
//============================================================================================================

void TestApp::OnGenerate (UIWidget* widget, const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft && !isDown)
	{
		mSeed = mRand.GenerateUint() + Time::GetMilliseconds();
		Generate();
	}
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