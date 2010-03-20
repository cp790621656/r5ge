#include "../Include/_All.h"

//============================================================================================================
// Sound Library
//============================================================================================================

using namespace R5;

Sound::~Sound()
{
	mData.Release();
}

ISoundInstance* Sound::Play (uint layer, float fadeInTime, bool repeat)
{
	SoundInstance* sound = 0;
	Lock();
	{
		if (mData.IsValid() || mData.Load(mName))
		{
			sound = (SoundInstance*)mAudio->Instantiate (this, layer, fadeInTime, repeat, mData);
			sound->Play();			
		}
	}
	Unlock();
	return sound;
}

// Play the specified sound at the specified position
ISoundInstance* Sound::Play (const Vector3f& position, uint layer , float fadeInTime, bool repeat)
{
	SoundInstance* sound = 0;
	Lock();
	{
		if (mData.IsValid() || mData.Load(mName))
		{
			sound = (SoundInstance*)mAudio->Instantiate (this, layer, fadeInTime, repeat, mData);
			sound->mIs3D = true;
			sound->Play();
		}
	}
	Unlock();
	return sound;
}

// Set the buffer used to create instances.
void Sound::Set(const byte* buffer, uint size)
{
	Lock();
	{
		mData.Set(buffer, size);	
	}
	Unlock();
}