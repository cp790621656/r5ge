#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Sound class to store specific sounds.
// Authors: Philip Cosgrave and Michael Lyashenko
//============================================================================================================

class Sound : public ISound
{
private:

	String				mName;		// File that is being referenced
	ALuint				mBuffer;	// The memory from which the instance is created
	IAudio*				mAudio;		// Audio instance which created this Sound
	Thread::Lockable	mLock;		// Makes the sound thread safe

	// Only the Audio class should be able to access SetAudio
	friend class Audio;

	void SetAudio  (IAudio* audio) { mAudio = audio;   }
	void SetSource (ALuint buffer)  { mBuffer = buffer; }

	// Get the sound buffer.
	ALuint GetBuffer() { return mBuffer; }
public:

	R5_DECLARE_ABSTRACT_CLASS("Sound", ISound);

	Sound (const String& name) :
			mName	(name),
			mBuffer (0),
			mAudio	(0) {}

	virtual ~Sound();

	// Get the name of the sound
	virtual const String& GetName()	 const { return mName;  }

	// Get the associated Audio class
	virtual IAudio* GetAudio() { return mAudio; }


	// Play the sound in 2D
	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

	// Play the specified sound at the specified position
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

protected:

	// Thread-safe functionality
	void Lock()		const { mLock.Lock(); }
	void Unlock()	const { mLock.Unlock(); }	
};
