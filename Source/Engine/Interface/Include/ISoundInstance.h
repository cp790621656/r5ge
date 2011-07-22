#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the SoundInstance class
// Author: Michael Lyashenko
//============================================================================================================
struct IAudioLayer;

struct ISoundInstance
{
	struct Effect
	{
		enum
		{
			None = 0,
			Auditorium,
		};
	};

	virtual ~ISoundInstance() {}
	
	virtual bool IsStopped()	const=0;
	virtual bool IsPaused()		const=0;

	// Destory the sound
	virtual void DestroySelf()=0;

	// Play the sound
	virtual void Play()=0;

	// Pause the sound
	virtual void Pause ()=0;

	// Stop the sound playback
	virtual void Stop ()=0;

	// Sets the 3D position of the specified sound
	virtual void SetPosition (const Vector3f& position)=0;
	
	// Sets the velocity of the specified sound
	virtual void SetVelocity (const Vector3f& velocity)=0;
	
	// Changes the volume of the specified sound
	virtual void SetVolume (float volume)=0;

	// Sets whether the sound will repeat after it ends
	virtual void SetRepeat (bool repeat)=0;

	// The effect that is going to be played on this sound
	virtual void SetEffect (byte effect)=0;

	// Gets the volume of the specified sound
	virtual float GetVolume () const=0;

	// Gets the layer of the specified sound
	virtual IAudioLayer* GetLayer () const=0;
};
