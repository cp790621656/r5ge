#pragma once

//============================================================================================================
//         R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Sound class to store specific sounds.
//============================================================================================================

class Sound : public ISound
{
private:

	String				mName;		// File that is being referenced
	Memory				mData;		// The memory from which the instance is created
	IAudio*				mAudio;		// Audio instance which created this Sound
	Thread::Lockable	mLock;		// Makes the sound thread safe

public:

	R5_DECLARE_ABSTRACT_CLASS("Sound", ISound);

	Sound (const String& name) :
			mName	(name),
			mAudio	(0) {}

	virtual ~Sound();

	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

	// Play the specified sound at the specified position
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false);

	// Set the buffer used to create instances.
	virtual void Set (const byte* buffer, uint size);

	virtual const IAudio* GetAudio() { return mAudio; }
	virtual const String& GetName()	 { return mName;  }
	
	virtual void SetAudio	(IAudio* audio)			{ mAudio = audio; }
	virtual void SetName	(const String& name)	{ mName = name;   }	

protected:

	// Thread-safe functionality
	void Lock()		const { mLock.Lock(); }
	void Unlock()	const { mLock.Unlock(); }	
};