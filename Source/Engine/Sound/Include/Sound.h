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

	String				mName;				// File that is being referenced
	uint				mBuffer;			// The memory from which the instance is created
	IAudio*				mAudio;				// Audio instance which created this Sound
	Thread::Lockable	mLock;				// Makes the sound thread safe
	mutable bool		mCheckForSource;	// Whether we should try to load the sound

	// Only the Audio class should be able to create sounds
	friend class Audio;
	Sound (IAudio* audio, const String& name) : mName (name), mBuffer (0), mAudio (audio), mCheckForSource(true) {}

public:

	R5_DECLARE_ABSTRACT_CLASS("Sound", ISound);

	virtual ~Sound();

	// Get the name of the sound
	virtual const String& GetName()	 const { return mName; }

	// Get the associated Audio class
	virtual IAudio* GetAudio() { return mAudio; }

	// Whether this is a playable sound
	virtual bool IsValid() const;

	// Set the source data from which the sound gets created
	virtual bool SetSourceData (const byte* buffer, uint size);

	// Play the sound in 2D
	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

	// Play the specified sound at the specified position
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);
};
