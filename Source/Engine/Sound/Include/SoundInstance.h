#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class AudioLayer;

class SoundInstance: public ISoundInstance
{
friend class Audio;

public:
	struct State
	{
		enum
		{
			Stopped = 0,
			Playing,
			Paused,
		};
	};

protected:
	Sound*		mSound;
	uint		mSource;
	uint		mState;
	float		mVolume;
	AudioLayer*	mLayer;

	LinkedList<SoundInstance*>::Entry*	mInstancesEntry;

	LinkedList<SoundInstance*>*			mActiveInstances;
	LinkedList<SoundInstance*>::Entry*	mActiveInstancesEntry;

protected:
	SoundInstance(){}

	SoundInstance(Sound* sound);
	virtual ~SoundInstance();

public:
	void SetSpatial(bool isSpatial);

	virtual void SetPosition(const Vector3f& position);
	virtual void SetVelocity(const Vector3f& velocity);

	virtual void SetRepeat(bool repeat)			= 0;
	virtual void Play()							= 0;
	virtual void Pause()						= 0;
	virtual void Stop()							= 0;
	virtual bool Update()						= 0;


	virtual void SetVolume(float volume);

	virtual float GetVolume() const
		{ return mVolume; }
	
	virtual bool IsStopped() const 
		{ return mState == State::Stopped; }

	virtual bool IsPaused() const 
		{ return mState == State::Paused; }

	virtual void DestroySelf()
		{ delete this; }

	virtual void SetEffect(byte) 
		{ /* TODO: Implement this */ }	
	
	virtual AudioLayer* GetLayer() const
		{ return mLayer; }
};
