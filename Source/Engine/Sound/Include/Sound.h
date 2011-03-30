#pragma once

//============================================================================================================
//         R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Sound class to store specific sounds.
//============================================================================================================

class Sound : public ISound
{
private:

	String				mName;		// File that is being referenced
	void*				mSource;	// The memory from which the instance is created
	IAudio*				mAudio;		// Audio instance which created this Sound
	Thread::Lockable	mLock;		// Makes the sound thread safe

	// Only the Audio class should be able to access SetAudio
	friend class Audio;

	void SetAudio  (IAudio* audio) { mAudio = audio;   }
	void SetSource (void* source)  { mSource = source; }

public:

	R5_DECLARE_ABSTRACT_CLASS("Sound", ISound);

	Sound (const String& name) :
			mName	(name),
			mAudio	(0), 
			mSource (0) {}

	virtual ~Sound();

	// Get the name of the sound
	virtual const String& GetName()	 const { return mName;  }

	// Get the associated Audio class
	virtual IAudio* GetAudio() { return mAudio; }

	// Get the source for the sound.
	virtual void* GetSource() { return mSource; }

	// Play the sound in 2D
	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

	// Play the specified sound at the specified position
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

protected:

	// Thread-safe functionality
	void Lock()		const { mLock.Lock(); }
	void Unlock()	const { mLock.Unlock(); }	
};