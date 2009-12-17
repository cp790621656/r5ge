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
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Scene			mScene;
	Mesh*			mMesh;
	uint			mSeed;
	Random			mRand;
	UISlider*		mSize;
	UISlider*		mTilt;
	UISlider*		mCount;
	bool			mIsDirty;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
	void Fill(Mesh::Vertices& verts, Mesh::Normals& normals, Mesh::TexCoords& tc, Mesh::Indices& indices);
	bool OnGenerate(UIArea* area, const Vector2i& pos, byte key, bool isDown);
	void Generate();
	void OnSerializeTo (TreeNode& node) const;
	bool OnSerializeFrom (const TreeNode& node);
};

//============================================================================================================

TestApp::TestApp() : mMesh(0), mSeed(0), mSize(0), mTilt(0), mCount(0), mIsDirty(true)
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
	mCore->SetListener( bind(&TestApp::OnSerializeTo,	this) );
	mCore->SetListener( bind(&TestApp::OnSerializeFrom, this) );
	mCore->SetListener( bind(&TestApp::OnDraw,			this) );

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

			mUI->SetOnKey("Generate", bind(&TestApp::OnGenerate, this));
		}

		// Model setup
		{
			mMesh = mCore->GetMesh("Branch");
			Model* model = mCore->GetModel("Branch");
			Limb* limb = model->GetLimb("Branch");
			limb->SetMesh(mMesh);
			limb->SetMaterial(mGraphics->GetMaterial("Leaf"));

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
			mScene.Cull(cam);

			Array<const ITechnique*> tech;
			tech.Expand() = mGraphics->GetTechnique("Transparent");

			static IRenderTarget* target0 = 0;
			static IRenderTarget* target1 = 0;

			static ITexture* diffuse = mGraphics->GetTexture("Out - Diffuse");
			static ITexture* normal  = mGraphics->GetTexture("Out - Normal");
			static ITexture* depth	 = mGraphics->GetTexture("Out - Depth");
			
			// Diffuse / normal draw
			{
				if (target0 == 0)
				{
					target0 = mGraphics->CreateRenderTarget();
					target0->AttachDepthTexture(depth);
					target0->AttachStencilTexture(depth);
					target0->AttachColorTexture(0, diffuse, ITexture::Format::RGBA);
					target0->AttachColorTexture(1, normal,  ITexture::Format::RGBA);
					target0->SetBackgroundColor( Color4ub(36, 56, 10, 0) );
					target0->SetSize( Vector2i(512, 512) );
				}

				mGraphics->SetActiveRenderTarget(target0);
				mGraphics->Clear();

				mGraphics->SetStencilTest(true);
				mGraphics->SetActiveStencilFunction ( IGraphics::Condition::Always, 0x1, 0x1 );
				mGraphics->SetActiveStencilOperation(
					IGraphics::Operation::Keep,
					IGraphics::Operation::Keep,
					IGraphics::Operation::Replace );

				mScene.Draw(tech);
				mGraphics->Flush();
			}

			// Ambient Occlusion
			{
				if (target1 == 0)
				{
					target1 = mGraphics->CreateRenderTarget();
					target1->AttachDepthTexture(depth);
					target1->AttachStencilTexture(depth);
					target1->AttachColorTexture(0, diffuse, ITexture::Format::RGBA);
					target1->SetSize(diffuse->GetSize());
					target1->SetBackgroundColor(target0->GetBackgroundColor());
				}

				/*SSAO::SetParams(3.0f, 6.0f);
				const ITexture* ao = SSAO::High(mGraphics, depth, normal);

				static IShader* bake = mGraphics->GetShader("PostProcess/BakeAO");
				static ITechnique* tech = mGraphics->GetTechnique("Post Process");

				mGraphics->SetActiveRenderTarget(target1);
				mGraphics->SetActiveStencilFunction ( IGraphics::Condition::Equal, 0x1, 0x1 );
				mGraphics->SetActiveStencilOperation(
					IGraphics::Operation::Keep,
					IGraphics::Operation::Keep,
					IGraphics::Operation::Keep );

				mGraphics->SetActiveProjection(IGraphics::Projection::Orthographic);
				mGraphics->SetActiveTechnique(tech);
				mGraphics->SetActiveMaterial(0);
				mGraphics->SetActiveShader(bake);
				mGraphics->SetActiveTexture(0, diffuse);
				mGraphics->SetActiveTexture(1, ao);
				mGraphics->Draw(IGraphics::Drawable::InvertedQuad);
				mGraphics->Flush();*/

				static IShader* bake = mGraphics->GetShader("PostProcess/Darken");
				static ITechnique* tech = mGraphics->GetTechnique("Post Process");

				mGraphics->SetActiveRenderTarget(target1);
				mGraphics->SetActiveStencilFunction ( IGraphics::Condition::Equal, 0x1, 0x1 );
				mGraphics->SetActiveStencilOperation(
					IGraphics::Operation::Keep,
					IGraphics::Operation::Keep,
					IGraphics::Operation::Keep );

				mGraphics->SetActiveProjection(IGraphics::Projection::Orthographic);
				mGraphics->SetActiveTechnique(tech);
				mGraphics->SetActiveMaterial(0);
				mGraphics->SetActiveShader(bake);
				mGraphics->SetActiveTexture(0, diffuse);
				mGraphics->SetActiveTexture(1, depth);
				mGraphics->Draw(IGraphics::Drawable::InvertedQuad);
				mGraphics->Flush();
			}

			UIWindow* final = FindWidget<UIWindow>(mUI, "Final Window");

			if (final != 0)
			{
				Vector2i size (diffuse->GetSize());
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

void TestApp::Fill (Mesh::Vertices& verts, Mesh::Normals& normals, Mesh::TexCoords& tc, Mesh::Indices& indices)
{
	mIsDirty = true;
	mRand.SetSeed(mSeed);

	// Maximum depth range
	float depth = 5.0f;

	// How much the leaves will twist
	float twist = HALFPI * 0.5f;

	// Maximum size of the particle
	float size = (mSize != 0 ? mSize->GetValue() * 0.5f : 0.2f);

	// Tilting bias -- higher means more tilting
	float bias = (mTilt != 0 ? mTilt->GetValue() : 0.5f);
	float invBias = 1.1f - bias;

	// Number of particles -- 1 to 500
	uint count = (mCount != 0 ? (uint)Float::Max(1.0f, mCount->GetValue() * 500.0f) : 100);

	// Maximum distance a particle can be spawned at without getting clipped
	float range = Float::Max(0.0f, 1.0f - size * 1.145f);

	Vector3f pos, axis;
	Quaternion rot;

	float rangeSq = range * range;

	for (uint b = 0; b < count; ++b)
	{
		do 
		{
			pos.Set(mRand.GenerateRangeFloat() * range,
					mRand.GenerateRangeFloat() * range,
					0.0f);
		}
		while ((pos.x * pos.x + pos.y * pos.y) > range);		

		pos.z = (depth * b) / count;

		axis.Set(mRand.GenerateRangeFloat() * bias, mRand.GenerateRangeFloat() * bias, invBias);
		axis.Normalize();

		rot.SetFromDirection( Vector3f(pos.x, pos.y, 0.0f) );
		rot *= Quaternion(axis, mRand.GenerateRangeFloat() * twist);

		verts.Expand().Set(-size,  size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		verts.Expand().Set(-size, -size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		verts.Expand().Set( size, -size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		verts.Expand().Set( size,  size, 0.0f);
		verts.Back() *= rot;
		verts.Back() += pos;

		tc.Expand().Set(0.0f, 1.0f);
		tc.Expand().Set(0.0f, 0.0f);
		tc.Expand().Set(1.0f, 0.0f);
		tc.Expand().Set(1.0f, 1.0f);

		Vector3f normal (rot.GetUp());
		for (uint i = 0; i < 4; ++i) normals.Expand() = normal;
	}

	for (uint i = 0; i < verts.GetSize(); ++i)
		indices.Expand() = (ushort)i;
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
				mMesh->GetNormalArray(),
				mMesh->GetTexCoordArray(),
				mMesh->GetIndexArray());

			mMesh->SetPrimitive(IGraphics::Primitive::Quad);
			mMesh->Update(true, true, true, true, false, true);
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