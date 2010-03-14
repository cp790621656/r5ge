#pragma once

//============================================================================================================
//         R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// cAudio-based audio manager
//============================================================================================================

class Audio : public IAudio
{
private:

	struct TargetAction
	{
		enum
		{
			None	= 0,
			Pause	= 1,
			Stop	= 2,
		};
	};

	struct Sound
	{
		String		mName;			// File that is being referenced
		Vector3f	mPosition;		// Fade in, play, and fade out duration
		Vector3f	mVolume;		// Volume the sound is at. X = start, Y = current, Z = end
		uint		mLayer;			// Layer this sound belongs to
		void*		mAudioSource;	// Pointer to the IAudioSource provided by cAudio
		float		mDuration;		// Fade duration
		ulong		mStart;			// The start time of the fade
		uint		mAction;		// What action will be executed when the fade completes

		Sound (const String& name) : 
			mName		(name),
			mVolume		(0.0f, 0.0f, 1.0f), 
			mLayer		(0),
			mAudioSource(0), 
			mDuration	(0.0f),
			mStart		(0),
			mAction		(TargetAction::None) {}
	};

	typedef Sound* SoundPtr;
	typedef Array<Sound*> PlayingSounds;

	struct AudioLayer
	{
		uint			mLayer;
		float			mVolume;
		PlayingSounds	mSounds;

		AudioLayer(uint layer, float volume) : mLayer(layer), mVolume(volume) {} 
	};

	typedef AudioLayer* AudioLayerPtr;

private:

	void*					mAudioLib;
	PointerHash<AudioLayer>	mLayers;
	PointerHash<Sound>		mLibrary;
	Thread::Lockable		mLock;

public:

	R5_DECLARE_INHERITED_CLASS("Audio", Audio, IAudio, IAudio);

	Audio();
	virtual ~Audio();

	// Thread-safe functionality
	void Lock()		const { mLock.Lock(); }
	void Unlock()	const { mLock.Unlock(); }

	// Release all audio resources
	virtual void Release();

	// Update notification
	virtual void Update();

	// Adds a new sound to the library
	virtual void Add (const String& name, const byte* buffer, uint size);

	// Play the specified sound in 2D
	virtual void Play (const String& name,
		uint	layer		= 0, 
		bool	repeat		= false, 
		float	volume		= 1.0f,
		float	fadeInTime	= 0.0f);

	// Play the specified sound at the specified position
	virtual void Play (const String& name, const Vector3f& position,
		uint	layer		= 0, 
		bool	repeat		= false, 
		float	volume		= 1.0f,
		float	fadeInTime	= 0.0f);

	// Pause the playback of the specified sound temporarily, fading it out over the specified duration
	virtual void Pause (const String& name, float duration = 0.25f);

	// Stop playing the specified sound, fading it out over the specified duration
	virtual void Stop (const String& name, float duration = 0.25f);

	// Release all resources associated with the specified sound
	virtual void Release (const String& name);

	// Sets the sound listener position (usually should be the camera's position)
	virtual void SetListenerPosition(const Vector3f& position);

	// Changes the volume of an entire sound layer (music layer, for example)
	virtual void SetLayerVolume (uint layer, float volume, float duration = 0.25f);

	// Changes the volume of the specified sound
	virtual void SetSoundVolume	(const String& name, float volume, float duration = 0.25f)
	{
		Lock();
		_SetVolume(_AddSound(name), volume, duration);
		Unlock();
	}

	// Sets whether the sound will repeat after it ends
	virtual void SetSoundRepeat (const String& name, bool repeat)
	{
		Lock();
		_SetRepeat(_AddSound(name), repeat);
		Unlock();
	}

	// Sets the 3D position of the specified sound
	virtual void SetSoundPosition (const String& name, const Vector3f& position)
	{
		Lock();
		_SetPosition(_AddSound(name), position);
		Unlock();
	}

	// Gets the volume of the specified layer
	virtual const float	GetLayerVolume (uint layer) const;

	// Gets the volume of the specified sound
	virtual const float	GetSoundVolume (const String& name) const;

private:

	AudioLayer* _GetAudioLayer (uint layer, float volume);
	Sound* _AddSound	(const String& fileName);
	Sound* _PlaySound	(const String& fileName, uint layer, bool repeat, float volume, float duration);

	void _Release		(Sound* sound);
	void _Pause			(Sound* sound, float duration = 0.25f);
	void _Stop			(Sound* sound, float duration = 0.25f);
	void _SetVolume		(Sound* sound, float volume, float duration = 0.25f);
	void _SetRepeat		(Sound* sound, bool loop);
	void _SetPosition	(Sound* sound, Vector3f position);
};