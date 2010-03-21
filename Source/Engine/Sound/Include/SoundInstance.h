#pragma once

//============================================================================================================
//          R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Instanced sound object
//============================================================================================================

class SoundInstance : public ISoundInstance
{
private:

	friend class Sound;
	friend class Audio;

	struct TargetAction
	{
		enum
		{
			None	= 0,
			Pause	= 1,
			Stop	= 2,
		};
	};

protected:

	Vector3f	mPosition;		// Fade in, play, and fade out duration
	Vector4f	mVolume;		// Volume the sound is at. X = start, Y = current, Z = end, W = volume unmodified
	bool		mRepeat;		// If this sound will repeat
	uint		mLayer;			// Layer this sound belongs to
	void*		mAudioSource;	// Pointer to the IAudioSource provided by cAudio
	float		mDuration;		// Fade duration
	ulong		mStart;			// The start time of the fade
	byte		mAction;		// What action will be executed when the fade completes
	bool		mIs3D;			// If this sound is being player in 3D
	ISound*		mSound;			// The sound this instance was created from

public:

	SoundInstance()	:
			mVolume		(0.0f, 0.0f, 1.0f, 1.0f), 
			mLayer		(0),
			mAudioSource(0), 
			mDuration	(0.0f),
			mStart		(0),
			mAction		(TargetAction::None),
			mIs3D		(false),
			mSound		(0) {}

	~SoundInstance(){}

	R5_DECLARE_ABSTRACT_CLASS("Sound Instance", ISoundInstance);

public:

	virtual const ISound*	GetSound()	const	{ return mSound; }
	virtual const bool		Is3D()		const	{ return mIs3D;	 }
	virtual const bool		IsPlaying()	const;
	virtual const bool		IsPaused()	const;

	// Destory the sound
	virtual void DestroySelf();

	// Play the sound
	virtual void Play();

	// Pause the sound
	virtual void Pause (float duration = 0.25f);

	// Stop the sound playback
	virtual void Stop (float duration = 0.25f);

	// Sets the 3D position of the specified sound
	virtual void SetPosition (const Vector3f& position);
	
	// Changes the volume of the specified sound
	virtual void SetVolume (float volume, float duration = 0.25f);

	// Sets whether the sound will repeat after it ends
	virtual void SetRepeat (bool repeat);

	// Gets the volume of the specified sound
	virtual const float	GetVolume () const { return mVolume.w; }

	// Gets the volume of the specified sound
	virtual const bool	GetRepeat() const { return mRepeat; }
	
	// Gets the volume of the specified sound
	virtual const Vector3f&	GetPosition() const { return mPosition; }

	// Gets the layer of the specified sound
	virtual const uint	GetLayer() const { return mLayer; }

private:

	void _SetVolume	(float volume, float calculatedVolume, float duration);
};