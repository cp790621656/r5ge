#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

ModelViewer::ModelViewer() : mCam(0), mModel(0), mInst(0), mAnimate(false), mSbLabel(0), mTimestamp(0), mResetCamera(false)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	// Event listeners
	mCore->SetListener( bind(&ModelViewer::OnDraw,			this) );
	mCore->SetListener( bind(&ModelViewer::OnKeyPress,		this) );
	mCore->SetListener( bind(&ModelViewer::OnMouseMove,		this) );
	mCore->SetListener( bind(&ModelViewer::OnScroll,		this) );
	mCore->SetListener( bind(&ModelViewer::SerializeFrom,	this) );
	mCore->SetListener( bind(&ModelViewer::SerializeTo,		this) );

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
	if (*mCore << "Config/Model Viewer.txt")
	{
		mCam			= mScene.FindObject<DebugCamera>	("Default Camera");
		mLight			= mScene.FindObject<Light>			("Default Light");
		mStage			= mScene.FindObject<Object>			("Stage");
		mInst			= mScene.FindObject<ModelInstance>	("Default Instance");
		mSbHighlight	= mUI->FindWidget<UIHighlight>		("Status Highlight");
		mSbLabel		= mUI->FindWidget<UILabel>			("Status Label");

		// Model viewer deals with only one model
		mModel = mCore->GetModel("Default Model");

		// If something wasn't found, just exit
		if (mCam == 0 || mLight == 0 || mStage == 0 || mInst == 0 || mModel == 0 || !CreateUI()) return;

		// Display the current version
		ShowAboutInfo();

		// Endless loop
		while ( mCore->Update() );

		// Reset the stage's rotation before saving the scene
		mStage->SetRelativeRotation( Quaternion() );

		// Save the current scene
		*mCore >> "Config/Model Viewer.txt";
	}
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

	// Prepare to draw the scene
	mScene.Cull(mCam);

	// If there was a request to reset the viewpoint, now should be a good time as we know what's visible
	if (mResetCamera)
	{
		mResetCamera = false;
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

	// Deferred rendering part
	{
		static Deferred::Storage::Techniques ft;

		if (ft.IsEmpty())
		{
			ft.Expand() = mGraphics->GetTechnique("Wireframe");
			ft.Expand() = mGraphics->GetTechnique("Glow");
			ft.Expand() = mGraphics->GetTechnique("Glare");
		}

		// Draw the scene using the deferred approach
		mScene.DrawAllDeferred(mParams.mSsao, 0);

		// Draw the grid and all forward-rendered objects last
		mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
		mGraphics->SetActiveTechnique(ft[0]);
		mGraphics->Draw( IGraphics::Drawable::Grid );

		// Draw the scene using the forward rendering approach, adding glow and glare effects
		mScene._Draw(ft);
	}

	// Post-processing part
	if (mParams.mBloom)
	{
		// Apply a post-processing effect
		PostProcess::Bloom(mGraphics, mScene, mParams.mThreshold);
		//PostProcess::DepthOfField(mGraphics, result.mColor, result.mDepth, 15.0f, 5.0f, 15.0f);
	}
	else
	{
		// Don't apply any effects -- simply draw this texture directly to the screen
		PostProcess::None(mGraphics, mScene);
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
		mSbLabel->SetColor(color);
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

bool ModelViewer::OnKeyPress(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::F5 )
		{
			_ToggleFullscreen();
		}
		else if ( key == Key::F6 )
		{
			_ResetWindow();
		}
		else if ( key == Key::F9 )
		{
			mResetCamera = true;
		}
		else if ( key == Key::Escape )
		{
			mCore->Shutdown();
		}
		else if (mCore->IsKeyDown(Key::LeftControl))
		{
			if ( key == Key::S )
			{
				OnFileSave();
			}
			else if ( key == Key::A )
			{
				ShowSaveAsDialog();
			}
			else if ( key == Key::O  )
			{
				ShowOpenDialog();
			}
		}
	}
	return true;
}

//============================================================================================================
// Responds to mouse movement
//============================================================================================================

bool ModelViewer::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
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
				}
			}
			else mCam->MouseMove(pos, delta);
		}
		else mCam->MouseMove(pos, delta);
	}
	return true;
}

//============================================================================================================
// Responds to scroll events
//============================================================================================================

bool ModelViewer::OnScroll(const Vector2i& pos, float delta)
{
	if (mCam) mCam->Scroll(pos, delta * 0.5f);
	return true;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool ModelViewer::SerializeFrom (const TreeNode& root)
{
	if (root.mTag == "File History")
	{
		if (root.mValue.IsStringArray())
		{
			mSavedHistory.Lock();
			mSavedHistory = root.mValue.AsStringArray();
			mSavedHistory.Unlock();
		}
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
	ModelViewer app;
	app.Run();
	return 0;
}