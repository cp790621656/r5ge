//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Dev11: Sound
//------------------------------------------------------------------------------------------------------------
// Sound testing
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Render, Sound
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Sound/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{
	#include "../Include/OSSound.h"
};

using namespace R5;

//============================================================================================================

class TestApp : Thread::Lockable
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;
	Audio*			mAudio;

	struct NamedSound
	{
		SoundInstance*	mSound;
		String			mName;

		NamedSound() : mSound(0) {}
	};

	PointerArray<NamedSound> mSounds;

public:

	TestApp();
	~TestApp();
	void Run();

	void OnVolumeChange (UIWidget* widget);
	void OnPlayClick	(UIWidget* widget, uint state, bool isSet);
	void OnStopClick	(UIWidget* widget, uint state, bool isSet);
	void OnPauseClick	(UIWidget* widget, uint state, bool isSet);
	void OnResumeClick	(UIWidget* widget, uint state, bool isSet);

	void OnLayerVolumeChange (UIWidget* widget);
	void OnLayerChanged		 (UIWidget* widget, bool focus);
};

//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI (mGraphics, mWin);
	mAudio		= new Audio();
	mCore		= new Core (mWin, mGraphics, mUI, mAudio);

	// NOTE: Ideally this should be done via scripts instead of like this
	mUI->SetOnValueChange ("Volume",		bind(&TestApp::OnVolumeChange,		this));
	mUI->SetOnStateChange ("Play",			bind(&TestApp::OnPlayClick,			this));
	mUI->SetOnStateChange ("Stop",			bind(&TestApp::OnStopClick,			this));
	mUI->SetOnStateChange ("Pause",			bind(&TestApp::OnPauseClick,		this));
	mUI->SetOnStateChange ("Resume",		bind(&TestApp::OnResumeClick,		this));
	mUI->SetOnValueChange ("Layer Volume",	bind(&TestApp::OnLayerVolumeChange,	this));
	mUI->SetOnFocus		  ("Layer Value",	bind(&TestApp::OnLayerChanged,		this));

	// Register the scripts
	Script::Register<OSSound>();
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
		while (mCore->Update())
		{
			Lock();
			for (uint i = mSounds.GetSize(); i > 0;)
			{
				NamedSound* namedSound = (NamedSound*)mSounds[--i];

				if (namedSound->mSound != 0)
				{
					if (namedSound->mSound->IsStopped())
					{
						namedSound->mSound->DestroySelf();
						mSounds.Remove(namedSound);
					}
				}
			}
			Unlock();
		}
	}
	//*mCore >> "Config/Dev11.txt";
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
			Lock();
			for (uint i = mSounds.GetSize(); i > 0;)
			{
				NamedSound* namedSound = (NamedSound*)mSounds[--i];

				if (namedSound->mName == sound->GetText())
				{
					if (namedSound->mSound != 0)
					{
						namedSound->mSound->SetVolume(val);
					}
					
					break;
				}
			}	
			Unlock();
		}		

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );
	}
}

//============================================================================================================

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

			ASSERT(checkBox != 0 && slider != 0 && layer != 0, "Missing a widget");

			Lock();
			if (sound)
			{
				NamedSound* namedSound = new NamedSound();
				uint layerVal;
				if (!(layer->GetText() >> layerVal)) layerVal = 0;

				namedSound->mSound = (SoundInstance*)mAudio->Instantiate
						(mAudio->GetSound(sound->GetText()), layerVal, 0.0f, (checkBox->GetState() & UIButton::State::Pressed) != 0);

				namedSound->mSound->Play();
				namedSound->mSound->SetVolume(slider->GetValue());
				namedSound->mName = sound->GetText();
				mSounds.Expand() = namedSound;				
			}
			Unlock();
		}
	}
}

//============================================================================================================

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
				Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];
					uint layerVal;

					if (!(layer->GetText() >> layerVal)) layerVal = 0;

					if (namedSound->mName == sound->GetText() && namedSound->mSound->GetLayer() == mAudio->GetLayer(layerVal))
					{
						if (namedSound->mSound != 0)
						{
							namedSound->mSound->Stop();
						}
						break;
					}
				}
				Unlock();
			}
		}
	}
}

//============================================================================================================

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
				Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];
					uint layerVal;

					if (!(layer->GetText() >> layerVal)) layerVal = 0;

					if (namedSound->mName == sound->GetText() && namedSound->mSound->GetLayer() == mAudio->GetLayer(layerVal))
					{
						if (namedSound->mSound != 0)
						{
							namedSound->mSound->Pause();
						}
						break;
					}
				}
				Unlock();
			}
		}
	}
}

//============================================================================================================

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
				Lock();
				for (uint i = mSounds.GetSize(); i > 0;)
				{
					NamedSound* namedSound = (NamedSound*)mSounds[--i];
					uint layerVal;

					if (!(layer->GetText() >> layerVal)) layerVal = 0;

					if (namedSound->mName == sound->GetText() && 
						namedSound->mSound->GetLayer() == mAudio->GetLayer(layerVal) && 
						namedSound->mSound->IsPaused())
					{
						if (namedSound->mSound != 0)
						{
							namedSound->mSound->Play();
						}
						break;
					}
				}
				Unlock();
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

		mAudio->GetLayer(layerVal)->SetVolume(val);

		UITextLine* txt = slider->FindWidget<UITextLine>(slider->GetName() + " Value", false);
		if (txt) txt->SetText( String("%.2f", val) );
	}
}

//============================================================================================================

void TestApp::OnLayerChanged (UIWidget* widget, bool focus)
{	
	if (!focus)
	{
		UIInput* layer = R5_CAST(UIInput, widget);
		UISlider* slider = mUI->FindWidget<UISlider>("Layer Volume");
		uint layerVal;

		if (!(layer->GetText() >> layerVal)) layerVal = 0;	

		slider->SetValue(mAudio->GetLayer(layerVal)->GetVolume());
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
