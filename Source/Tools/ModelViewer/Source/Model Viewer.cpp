#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

ModelViewer::ModelViewer() : mScene(0), mDraw(0), mCam(0), mModel(0), mInst(0),
	mAnimate(false), mSbLabel(0), mTimestamp(0), mResetCamera(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	// Framerate update function
	mCore->AddOnPostUpdate( bind(&ModelViewer::UpdateFPS, this) );
}

//============================================================================================================

ModelViewer::~ModelViewer()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void ModelViewer::Run()
{
	TreeNode root;

	// Create the environment cube map
	{
		Noise noise;
		noise.SetSeed(7654321);
		noise.SetSize(32, 32);
		noise.SetSeamless(false);
		noise.ApplyFilter("Simple");
		void* ptr = noise.GetBuffer();

		ITexture* tex = mGraphics->GetTexture("Environment Map");
		tex->Set(ptr, ptr, ptr, ptr, ptr, ptr, 32, 32, ITexture::Format::Float, ITexture::Format::Alpha);
		tex->SetWrapMode(ITexture::WrapMode::ClampToEdge);
	}

	// Create the noise map
	{
		Noise noise;
		noise.SetSeed(74625646);
		noise.SetSize(32, 32);
		noise.SetSeamless(true);
		noise.ApplyFilter("Perlin").Set(3.0f);

		ITexture* tex = mGraphics->GetTexture("Noise map");
		tex->Set(noise.GetBuffer(), 32, 32, 1, ITexture::Format::Float, ITexture::Format::Alpha);
		tex->SetWrapMode(ITexture::WrapMode::Repeat);
	}

	if (!root.Load("Config/Model Viewer.txt")) return;
	
	mCore->Lock();
	mCore->SerializeFrom(root);
	mCore->Unlock();

	// Event listeners
	mCore->AddOnKey		 ( bind(&ModelViewer::OnKeyPress,	this) );
	mCore->AddOnMouseMove( bind(&ModelViewer::OnMouseMove,	this) );
	mCore->AddOnScroll	 ( bind(&ModelViewer::OnScroll,		this) );
	mCore->AddOnDraw	 ( bind(&ModelViewer::OnDraw,		this) );

	Object* rootObj = mCore->GetRoot();

	mCam			= rootObj->FindObject<DebugCamera>	("Default Camera");
	mLight			= rootObj->FindObject<Light>		("Default Light");
	mStage			= rootObj->FindObject<Object>		("Stage");
	mInst			= rootObj->FindObject<ModelInstance>("Default Instance");
	mSbHighlight	= mUI->FindWidget<UIHighlight>		("Status Highlight");
	mSbLabel		= mUI->FindWidget<UILabel>			("Status Label");

	// Model viewer deals with only one model
	mModel = mCore->GetModel("Default Model");

	// If something wasn't found, just exit
	if (mCam == 0 || mLight == 0 || mStage == 0 || mInst == 0 || mModel == 0 || !CreateUI()) return;

	// Deferred drawing
	mDraw  = mCam->AddScript<OSDrawDeferred>();
	mScene = &mDraw->GetScene();

	// Display the current version
	ShowAboutInfo();

	// Endless loop
	while ( mCore->Update() );

#ifndef _DEBUG
	// Reset the stage's rotation before saving the scene
	mStage->SetRelativeRotation( Quaternion() );

	// Save the current scene
	root.Release();
	mCore->SerializeTo(root);
	root.Save("Config/Model Viewer.txt");
#endif
}

//============================================================================================================
// Main draw function called by the R5::Core
//============================================================================================================

void ModelViewer::OnDraw()
{
	// Animate the model if requested
	if (mAnimate)
	{
		ulong delta = Time::GetDeltaMS();

		if (delta > 0)
		{
			const Vector3f axis (0.0f, 0.0f, 1.0f);
			Quaternion rot (axis, -Float::Sin(0.0005f * delta));
			mStage->SetRelativeRotation( rot * mStage->GetRelativeRotation() );
		}
	}

	// Fade out the status bar
	if (mSbHighlight != 0 && mTimestamp != 0.0f)
	{
		float current = Time::GetTime();
		float factor = (current - mTimestamp) / 2.0f;
		factor = Float::Clamp(factor, 0.0f, 1.0f);
		mSbHighlight->SetAlpha( (1.0f - factor) * 0.85f );
		if (factor == 1.0f) mTimestamp = 0.0f;
	}

	// If there was a request to reset the viewpoint, now should be a good time as we know what's visible
	if (mResetCamera > 0 && mResetCamera++ > 1)
	{
		mResetCamera = 0;
		const Bounds& bounds = mInst->GetAbsoluteBounds();

		if (bounds.IsValid())
		{
			const Vector3f& center (bounds.GetCenter());
			Vector3f total = bounds.GetMax() - bounds.GetMin();
			total.Normalize();

			// Default positions should always be at a downward angle
			Vector3f down	  (0.175f, -0.5f, -1.0f );
			Vector3f straight ( 0.35f, -1.0f, -0.35f);

			down.Normalize();
			straight.Normalize();

			// How much the camera will be angled downward depends on the dot product
			float dot = total.Dot( Vector3f(0.0f, 0.0f, 1.0f) );
			Vector3f dir ( Interpolation::Linear(down, straight, dot) );

			// Distance should be far enough away to view the entire model
			float distance = Float::Clamp(bounds.GetRadius() * 1.2f, 1.0f, mCam->GetDolly().z);

			// Animate the camera to the calculated position
			mCam->Stop();
			mCam->AnimateTo(center, dir, distance, 0.5f);
		}
	}
}

//============================================================================================================
// Update the framerate every 250 milliseconds
//============================================================================================================

float ModelViewer::UpdateFPS()
{
	static UILabel* fps = mUI->FindWidget<UILabel>("FPS");
	if (fps != 0) fps->SetText( String("%u", Time::GetFPS()) );
	return 0.25f;
}

//============================================================================================================
// Changes the status bar text
//============================================================================================================

void ModelViewer::SetStatusText (const String& text, const Color3f& color)
{
	if (mSbLabel != 0)
	{
		mSbLabel->SetText(text);
		mSbLabel->SetTextColor(color);
		mSbLabel->BringToFront();
	}

	if (mSbHighlight != 0)
	{
		mTimestamp = Time::GetTime() + 5.0f;
	}
}

//============================================================================================================
// Responds to keypresses
//============================================================================================================

uint ModelViewer::OnKeyPress(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::F5 )
		{
			_ToggleFullscreen();
			return 1;
		}
		else if ( key == Key::F6 )
		{
			_ResetWindow();
			return 1;
		}
		else if ( key == Key::F9 )
		{
			mResetCamera = 1;
			return 1;
		}
		else if ( key == Key::Escape )
		{
			mCore->Shutdown();
			return 1;
		}
		else if (mCore->IsKeyDown(Key::LeftControl))
		{
			if ( key == Key::S )
			{
				OnFileSave();
				return 1;
			}
			else if ( key == Key::A )
			{
				ShowSaveAsDialog();
				return 1;
			}
			else if ( key == Key::O  )
			{
				ShowOpenDialog();
				return 1;
			}
		}
	}
	return 0;
}

//============================================================================================================
// Responds to mouse movement
//============================================================================================================

uint ModelViewer::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	if (mCam != 0)
	{
		if (mCore->IsKeyDown( Key::MouseLeft ))
		{
			if ( mCore->IsKeyDown(Key::L) )
			{
				if (mLight != 0)
				{
					const Quaternion& rot ( mLight->GetRelativeRotation() );
					Quaternion hori ( Vector3f(0.0f, 0.0f, 1.0f), 0.005f * delta.x );
					Quaternion vert ( Vector3f(1.0f, 0.0f, 0.0f), 0.005f * delta.y );
					mLight->SetRelativeRotation( hori * rot * vert );
					return 1;
				}
			}
			else if ( mCore->IsKeyDown(Key::M) )
			{
				if (mStage != 0)
				{
					const Quaternion& rot ( mStage->GetRelativeRotation() );
					Quaternion hori ( Vector3f(0.0f, 0.0f, 1.0f), -0.005f * delta.x );
					Quaternion vert ( Vector3f(1.0f, 0.0f, 0.0f), -0.005f * delta.y );
					mStage->SetRelativeRotation( hori * rot * vert );
					return 1;
				}
			}
			else return mCam->MouseMove(pos, delta);
		}
		else return mCam->MouseMove(pos, delta);
	}
	return 0;
}

//============================================================================================================
// Responds to scroll events
//============================================================================================================

uint ModelViewer::OnScroll(const Vector2i& pos, float delta)
{
	if (mCam) return mCam->Scroll(pos, delta * 0.5f);
	return 0;
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
//#ifdef _DEBUG
//	System::SetCurrentPath("../../DotC/Resources/");
//#endif
	ModelViewer app;
	app.Run();
	return 0;
}