//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
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
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;
	Scene		mScene;
	Mesh*		mMesh;
	uint		mSeed;
	Random		mRand;
	bool		mIsDirty;
	
	ITexture*	mDiffuseTex;
	ITexture*	mNormalTex;
	ITexture*	mDepthTex;

	UISlider*	mSize;
	UISlider*	mTilt;
	UISlider*	mCount;
	UIButton*	mAlpha;
	UIButton*	mNormal;
	UIPicture*	mFinal;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
	void Fill(Mesh::Vertices& verts, Mesh::TexCoords& tc, Mesh::Colors& colors);
	bool OnStateChange(UIArea* area);
	bool OnGenerate(UIArea* area, const Vector2i& pos, byte key, bool isDown);
	void Generate();
	void OnSerializeTo (TreeNode& node) const;
	bool OnSerializeFrom (const TreeNode& node);
};

//============================================================================================================

TestApp::TestApp() : mMesh(0), mSeed(0), mIsDirty(true), mDiffuseTex(0), mNormalTex(0), mDepthTex(0),
	mSize(0), mTilt(0), mCount(0), mAlpha(0), mNormal(0), mFinal(0)
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
	// Core event listeners
	mCore->SetListener( bind(&TestApp::OnSerializeTo,	this) );
	mCore->SetListener( bind(&TestApp::OnSerializeFrom, this) );
	mCore->SetListener( bind(&TestApp::OnDraw,			this) );

	// UI callbacks
	mUI->SetOnKey		 ("Generate",	bind(&TestApp::OnGenerate,		this));
	mUI->SetOnStateChange("Alpha",		bind(&TestApp::OnStateChange,	this));
	mUI->SetOnStateChange("Normal",		bind(&TestApp::OnStateChange,	this));

	// Textures we'll be using
	mDiffuseTex = mGraphics->GetTexture("Out - Diffuse");
	mNormalTex  = mGraphics->GetTexture("Out - Normal");
	mDepthTex	= mGraphics->GetTexture("Out - Depth");

	// Load the configuration
	if (*mCore << "Config/Dev9.txt")
	{
		// User Interface setup
		{
			ITexture* diffuse = mGraphics->GetTexture("Leaf Diffuse map");
			ITexture* normal  = mGraphics->GetTexture("Leaf Normal map");

			const Vector2i& sd = diffuse->GetSize();
			const Vector2i& sn = normal->GetSize();

			UIWindow* win0 = FindWidget<UIWindow>(mUI, "Leaf Diffuse Window");
			UIWindow* win1 = FindWidget<UIWindow>(mUI, "Leaf Normal Window");
			
			if (win0 != 0)
			{
				win0->ResizeToFit(sd);
				win0->SetText( String("Diffuse map (%ux%u)", sd.x, sd.y) );
			}

			if (win1 != 0)
			{
				win1->ResizeToFit(sn);
				win1->SetText( String("Normal map (%ux%u)", sn.x, sn.y) );
			}

			mSize	= FindWidget<UISlider>(mUI, "Size");
			mTilt	= FindWidget<UISlider>(mUI, "Tilt");
			mCount	= FindWidget<UISlider>(mUI, "Count");
			mAlpha	= FindWidget<UIButton>(mUI, "Alpha");
			mNormal = FindWidget<UIButton>(mUI, "Normal");
			mFinal	= FindWidget<UIPicture>(mUI, "Final");
		}

		// Model setup
		{
			mMesh = mCore->GetMesh("Branch");
			Model* model = mCore->GetModel("Branch");

			// Set the mesh/material
			model->GetLimb("Branch")->Set(mMesh, mGraphics->GetMaterial("Leaf"));

			ModelInstance* inst = AddObject<ModelInstance>(mScene, "Branch");
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

void TestApp::OnDraw()
{
	// Cull and draw the scene into an off-screen buffer
	if (mIsDirty)
	{
		mIsDirty = false;

		DebugCamera* cam = FindObject<DebugCamera>(mScene, "Default Camera");

		if (cam != 0)
		{
			Array<const ITechnique*> tech;
			tech.Expand() = mGraphics->GetTechnique("Transparent");

			static IRenderTarget* drawTarget = mGraphics->CreateRenderTarget();
			static IRenderTarget* postTarget = 0;

			mGraphics->SetStencilTest(false);
			mGraphics->SetDepthWrite(true);
			mGraphics->SetDepthTest(false);

			// Common properties
			drawTarget->AttachDepthTexture(mDepthTex);
			drawTarget->AttachStencilTexture(mDepthTex);
			drawTarget->AttachColorTexture(1, 0);
			drawTarget->SetSize( Vector2i(512, 512) );

			// Diffuse-only render target
			drawTarget->AttachColorTexture(0, mDiffuseTex, ITexture::Format::RGBA);
			drawTarget->SetBackgroundColor( Color4ub(36, 56, 10, 0) );
			
			// Clear depth, stencil, and diffuse
			mGraphics->SetActiveRenderTarget(drawTarget);
			mGraphics->Clear(true, true, true);

			// Unbind the render target
			mGraphics->SetActiveRenderTarget(0);

			// Normal map-only render target
			drawTarget->AttachColorTexture(0, mNormalTex, ITexture::Format::RGBA);
			drawTarget->SetBackgroundColor( Color4ub(127, 127, 255, 220) );
			
			// Clear the normal map
			mGraphics->SetActiveRenderTarget(drawTarget);
			mGraphics->Clear(true, false, false);

			// Unbind the render target
			mGraphics->SetActiveRenderTarget(0);

			// Set up the render targets for actual rendering
			drawTarget->AttachColorTexture(0, mDiffuseTex, ITexture::Format::RGBA);
			drawTarget->AttachColorTexture(1, mNormalTex, ITexture::Format::RGBA);

			// Activate the render target
			mGraphics->SetActiveRenderTarget(drawTarget);

			// Cull the scene
			mScene.Cull(cam);

			// Use the stencil buffer to speed things up
			mGraphics->SetStencilTest(true);
			mGraphics->SetActiveStencilFunction ( IGraphics::Condition::Always, 0x1, 0x1 );
			mGraphics->SetActiveStencilOperation(
				IGraphics::Operation::Keep,
				IGraphics::Operation::Keep,
				IGraphics::Operation::Replace );

			// Draw the scene into the render targets
			mScene.Draw(tech);

			UIWindow* final = FindWidget<UIWindow>(mUI, "Final Window");

			if (final != 0)
			{
				Vector2i size (mDiffuseTex->GetSize());
				final->ResizeToFit(size);
				final->SetText( String("Final Texture (%ux%u)", size.x, size.y) );
			}
		}
	}

	// Clear the screen
	mGraphics->SetActiveRenderTarget(0);
	mGraphics->Clear();
}

//============================================================================================================

void TestApp::Fill (Mesh::Vertices& verts, Mesh::TexCoords& tc, Mesh::Colors& colors)
{
	mIsDirty = true;
	mRand.SetSeed(mSeed);

	// Maximum depth range
	float depth = 5.0f;
	float invDepth = 1.0f / depth;

	// How much the leaves will twist
	float twist = HALFPI;

	// Maximum size of the particle
	float size = (mSize != 0 ? mSize->GetValue() * 0.5f : 0.2f);

	// Tilting bias -- higher means more tilting
	float bias = (mTilt != 0 ? mTilt->GetValue() : 0.5f);
	float invBias = 1.1f - bias;

	// Number of particles -- 1 to 500
	uint count = (mCount != 0 ? (uint)Float::Max(1.0f, mCount->GetValue() * 500.0f) : 100);

	// Maximum distance a particle can be spawned at without getting clipped
	float range = Float::Max(0.0f, 1.0f - size * 1.145f);

	// Color of the far-away vertices will be tinted up by up to this amount (1.0 - tint)
	float tint = 0.85f;
	float minTint = 1.0f - tint;

	Vector3f pos, axis;
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
		colors.Expand() = Color4f(shade, shade, shade, 1.0);

		// Bottom-left
		verts.Expand().Set(-size, -size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade, shade, shade, 1.0);

		// Bottom-right
		verts.Expand().Set( size, -size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade, shade, shade, 1.0);

		// Top-right
		verts.Expand().Set( size,  size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		shade = minTint + tint * verts.Back().z * invDepth;
		colors.Expand() = Color4f(shade, shade, shade, 1.0);

		// Texture coordinates are always the same
		tc.Expand().Set(0.0f, 1.0f);
		tc.Expand().Set(0.0f, 0.0f);
		tc.Expand().Set(1.0f, 0.0f);
		tc.Expand().Set(1.0f, 1.0f);
	}
}

//============================================================================================================

bool TestApp::OnStateChange(UIArea* area)
{
	if (mFinal != 0)
	{
		const ITexture* tex = (mNormal != 0 && mNormal->GetState(UIButton::State::Pressed)) ?
			mNormalTex : mDiffuseTex;

		bool alpha = (tex == mNormalTex) || (mAlpha != 0 && !mAlpha->GetState(UIButton::State::Pressed));

		mFinal->IgnoreAlpha(alpha);
		mFinal->SetTexture(tex);
	}
	return true;
}

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

void TestApp::Generate()
{
	if (mMesh != 0)
	{
		mMesh->Lock();
		{
			mMesh->Clear();

			Fill(mMesh->GetVertexArray(),
				 mMesh->GetTexCoordArray(),
				 mMesh->GetColorArray());

			mMesh->SetPrimitive(IGraphics::Primitive::Quad);
			mMesh->Update(true, true, true, true, false, false);
		}
		mMesh->Unlock();
	}
}

//============================================================================================================

void TestApp::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Seed", mSeed);
}

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