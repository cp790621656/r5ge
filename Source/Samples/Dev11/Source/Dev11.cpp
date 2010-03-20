//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev11: Sound
//------------------------------------------------------------------------------------------------------------
// Sound testing
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Sound/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

using namespace R5;

R5::Random randomGen;

//============================================================================================================

class TestApp
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Audio*			mAudio;
	Scene			mScene;
	DebugCamera*	mCam;

	struct NamedSound
	{
		SoundInstance*	mSound;
		String			mName;
	};

	PointerArray<NamedSound> mSounds;
	Thread::Lockable mLock;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();

	void OnVolumeChange (UIWidget* widget);
	void OnPlayClick	(UIWidget* widget, uint state, bool isSet);
	void OnStopClick	(UIWidget* widget, uint state, bool isSet);
	void OnPauseClick	(UIWidget* widget, uint state, bool isSet);
	void OnResumeClick	(UIWidget* widget, uint state, bool isSet);

	void OnLayerVolumeChange (UIWidget* widget);
	void OnLayerChanged		 (UIWidget* widget, bool focus);
};

//============================================================================================================

TestApp::TestApp() : mCam (0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI (mGraphics, mWin);
	mAudio		= new Audio();
	mCore		= new Core (mWin, mGraphics, mUI, mScene, mAudio);

	mUI->SetOnValueChange ("Volume",		bind(&TestApp::OnVolumeChange,		this));
	mUI->SetOnStateChange ("Play",			bind(&TestApp::OnPlayClick,			this));
	mUI->SetOnStateChange ("Stop",			bind(&TestApp::OnStopClick,			this));
	mUI->SetOnStateChange ("Pause",			bind(&TestApp::OnPauseClick,		this));
	mUI->SetOnStateChange ("Resume",		bind(&TestApp::OnResumeClick,		this));

	mUI->SetOnValueChange ("Layer Volume",	bind(&TestApp::OnLayerVolumeChange,	this));
	mUI->SetOnFocus		  ("Layer Value",	bind(&TestApp::OnLayerChanged,		this));
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mAudio)		delete mAudio;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/Dev11.txt")
	{
		mCam = mScene.FindObject<DebugCamera>("Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener (bind(&TestApp::OnDraw, this) );
			mCore->SetListener (bind(&Object::MouseMove, mCam) );
			mCore->SetListener (bind(&Object::Scroll, mCam) );

			while (mCore->Update())
			{
				mLock.Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];

					if (namedSound->mSound != 0)
					{
						if (!namedSound->mSound->IsPlaying() && !namedSound->mSound->IsPaused())
						{
							namedSound->mSound->DestroySelf();
							mSounds.Remove(namedSound);
						}
					}
				}
				mLock.Unlock();
			}
		}
	}
	//*mCore >> "Config/Dev11.txt";
}

//============================================================================================================
// Scene::Draw()
//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);
	Deferred::DrawResult result = mScene.DrawAllDeferred(0, 0);
	mScene.DrawAllForward(false);
	PostProcess::Bloom(mGraphics, result.mColor, 1.0f);
}


//============================================================================================================
// Sound functions
//============================================================================================================

void TestApp::OnVolumeChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		UIInput* sound = mUI->FindWidget<UIInput>("Sound Name");
		float val = slider->GetValue();

		if (sound)
		{
			mLock.Lock();
			for (uint i = mSounds.GetSize(); i > 0;)
			{
				NamedSound* namedSound = (NamedSound*)mSounds[--i];

				if (namedSound->mName == sound->GetText())
				{
					if (namedSound->mSound != 0)
					{
						namedSound->mSound->SetVolume(val, 0.0f);
					}
					
					break;
				}
			}	
			mLock.Unlock();
		}		

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );
	}
}

void TestApp::OnPlayClick (UIWidget* widget, uint state, bool isSet)
{
	UIButton* btn = R5_CAST(UIButton, widget);

	if (btn)
	{
		if (btn->GetState() & UIButton::State::Pressed)
		{
			UIInput*	sound		= mUI->FindWidget<UIInput>("Sound Name");
			UICheckbox* checkBox	= mUI->FindWidget<UICheckbox>("Repeat Checkbox");
			UISlider*	slider		= mUI->FindWidget<UISlider>("Volume");
			UIInput*	layer		= mUI->FindWidget<UIInput>("Layer");

			mLock.Lock();
			if (sound)
			{
				NamedSound* namedSound = new NamedSound();
				uint layerVal;
				if (!(layer->GetText() >> layerVal)) layerVal = 0;

				namedSound->mSound = (SoundInstance*)mAudio->GetSound(sound->GetText())->Play(layerVal, 0.0f, (checkBox->GetState() & UIButton::State::Pressed) != 0);
				namedSound->mSound->SetVolume(slider->GetValue());
				namedSound->mName = sound->GetText();
				mSounds.Expand() = namedSound;				
			}
			mLock.Unlock();
		}
	}
}

void TestApp::OnStopClick (UIWidget* widget, uint state, bool isSet)
{
	UIButton* btn = R5_CAST(UIButton, widget);

	if (btn)
	{
		if (btn->GetState() & UIButton::State::Pressed)
		{
			UIInput* sound = mUI->FindWidget<UIInput>("Sound Name");
			UIInput* layer = mUI->FindWidget<UIInput>("Layer");
			
			if (sound)
			{
				mLock.Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];
					uint layerVal;

					if (!(layer->GetText() >> layerVal)) layerVal = 0;

					if (namedSound->mName == sound->GetText() && namedSound->mSound->GetLayer() == layerVal)
					{
						if (namedSound->mSound != 0)
						{
							namedSound->mSound->Stop();
						}
						break;
					}
				}
				mLock.Unlock();
			}
		}
	}
}

void TestApp::OnPauseClick (UIWidget* widget, uint state, bool isSet)
{
	UIButton* btn = R5_CAST(UIButton, widget);

	if (btn)
	{
		if (btn->GetState() & UIButton::State::Pressed)
		{
			UIInput* sound = mUI->FindWidget<UIInput>("Sound Name");
			UIInput* layer = mUI->FindWidget<UIInput>("Layer");

			if (sound)
			{
				mLock.Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];
					uint layerVal;

					if (!(layer->GetText() >> layerVal)) layerVal = 0;

					if (namedSound->mName == sound->GetText() && namedSound->mSound->GetLayer() == layerVal)
					{
						if (namedSound->mSound != 0)
						{
							namedSound->mSound->Pause();
						}
						break;
					}
				}
				mLock.Unlock();
			}
		}
	}
}

void TestApp::OnResumeClick (UIWidget* widget, uint state, bool isSet)
{
	UIButton* btn = R5_CAST(UIButton, widget);

	if (btn)
	{
		if (btn->GetState() & UIButton::State::Pressed)
		{
			UIInput* sound = mUI->FindWidget<UIInput>("Sound Name");
			UIInput* layer = mUI->FindWidget<UIInput>("Layer");

			if(sound)
			{
				mLock.Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];
					uint layerVal;

					if (!(layer->GetText() >> layerVal)) layerVal = 0;

					if (namedSound->mName == sound->GetText() && namedSound->mSound->GetLayer() == layerVal && namedSound->mSound->IsPaused())
					{
						if (namedSound->mSound != 0)
						{
							namedSound->mSound->Play();
						}
						break;
					}
				}
				mLock.Unlock();
			}
		}
	}
}

//============================================================================================================
// Layer functions
//============================================================================================================

void TestApp::OnLayerVolumeChange (UIWidget* widget)
{
	UISlider* slider = R5_CAST(UISlider, widget);

	if (slider)
	{
		UIInput* layer = mUI->FindWidget<UIInput>("Layer Value");
		float val = slider->GetValue();
		uint layerVal;

		if (!(layer->GetText() >> layerVal)) layerVal = 0;	

		mAudio->SetLayerVolume(layerVal, val);

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );
	}
}
void TestApp::OnLayerChanged (UIWidget* widget, bool focus)
{	
	if (!focus)
	{
		UIInput* layer = R5_CAST(UIInput, widget);
		UISlider* slider = mUI->FindWidget<UISlider>("Layer Volume");
		uint layerVal;

		if (!(layer->GetText() >> layerVal)) layerVal = 0;	

		slider->SetValue(mAudio->GetLayerVolume(layerVal));
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