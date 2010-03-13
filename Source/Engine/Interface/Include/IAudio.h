#pragma once

//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic interface for the Audio class
//============================================================================================================

struct IAudio
{
	R5_DECLARE_INTERFACE_CLASS("Audio");

	struct Event
	{
		enum
		{
			NONE	= 0,
			PAUSE	= 1,
			STOP	= 2
		};
	};

	struct Sound
	{
		String		mFileName;		// File that is being referenced
		Vector3f	mPosition;		// Fade in, play, and fade out duration
		uint		mLayer;			// Layer this animation is activated on
		bool		mLoop;			// Whether to loop this animation
		Vector3f	mVolume;		// Volume the sound is at. X = start, Y = current Z = end
		void*		mAudioSource;	// Pointer to the IAudioSource provided by CAudio
		float		mDuration;		// Fade duration
		ulong		mStart;			// The start time of the fade
		uint		mEvent;			// What action is currently being executed on the Audio

		Sound() : 
			mVolume (0.0f, 0.0f, 1.0f), 
			mPosition (0.0f, 0.0f, 0.0f), 
			mLoop (false), 
			mAudioSource (0), 
			mEvent (0){}
	};

public:

	virtual ~IAudio() {}

	virtual void Update() = 0;
	
	virtual void Play (
		String& fileName, 
		uint layer = 0, 
		bool loop = false, 
		float volume = 1.0f,
		float duration = 0.0f) = 0;

	virtual void Play (
		String& fileName,
		Vector3f position,
		uint layer = 0, 
		bool loop = false, 
		float volume = 1.0f,
		float duration = 0.0f) =0;

	virtual void	Stop		(String& fileName, float duration = 0.25f) = 0;
	virtual void	Pause		(String& fileName, float duration = 0.25f) = 0;
	virtual void	Release		(String& fileName) = 0;
	
	virtual void	SetSoundVolume		(String& fileName, float volume, float duration = 0.25f)= 0;
	virtual void	SetLayerVolume		(uint layer, float volume, float duration = 0.25f) = 0;
	virtual void	SetListenerPosition (const Vector3f position) = 0;
	virtual void	SetSoundLooping		(String& fileName, bool loop) = 0;
	virtual void	SetSoundPosition	(String& fileName, Vector3f position) = 0;

	virtual const float	GetLayerVolume	(uint layer) const = 0;
	virtual const float	GetSoundVolume	(String& fileName) const = 0;
};