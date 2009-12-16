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
	DebugCamera*	mCam;
	Mesh*			mMesh;
	uint			mSeed;
	Random			mRand;
	UISlider*		mSize;
	UISlider*		mTilt;
	UISlider*		mCount;

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

TestApp::TestApp() : mCam(0), mMesh(0), mSeed(0), mSize(0), mTilt(0), mCount(0)
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

		// SSAO parameters -- shortened focus range and increased strength
		SSAO::SetParams(1.0f, 8.0f);

		mCam = FindObject<DebugCamera>(mScene, "Default Camera");

		mCore->SetListener( bind(&TestApp::OnDraw, this) );
		mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
		mCore->SetListener( bind(&Camera::OnScroll, mCam) );

		while (mCore->Update()) {}

		*mCore >> "Config/Dev9.txt";
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	mScene.DrawAllDeferred(true, false);
}

//============================================================================================================

void TestApp::Fill (Mesh::Vertices& verts, Mesh::Normals& normals, Mesh::TexCoords& tc, Mesh::Indices& indices)
{
	mRand.SetSeed(mSeed);

	// Maximum depth range
	float depth = 1.0f;

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

	for (uint b = 0; b < count; ++b)
	{
		rot.SetFromAxisAngle(Vector3f(0.0f, 0.0f, 1.0f), mRand.GenerateRangeFloat() * PI);
		float val = mRand.GenerateFloat();
		pos = rot.GetForward() * (range * (1.0f - val * val));
		pos.z += mRand.GenerateRangeFloat() * depth;

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