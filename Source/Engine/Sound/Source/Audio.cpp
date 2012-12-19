#include "../Include/_All.h"

#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"

using namespace R5;

AudioLibrary g_audioLibrary;

Audio::Audio()
{
	ALCdevice* device = alcOpenDevice(NULL);
	ASSERT(device != 0, "Couldn't open the default OpenAL device");
	
	ALCcontext* context = alcCreateContext(device, NULL);
	ASSERT(context != 0, "Couldn't create the default context");

	ALCboolean retval = alcMakeContextCurrent(context);
	ASSERT(retval != 0, "Couldn't make the context current");
}

Audio::~Audio()
{
	Release();
	ALCcontext* context = alcGetCurrentContext();
	ALCdevice* device = alcGetContextsDevice(context);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void Audio::Release()
{
	g_audioLibrary.Lock();
	{
		mLayers.Release();

		if (mSounds.IsValid())
		{
			LinkedList<Sound*>::Entry* nextEntry;
			LinkedList<Sound*>::Entry* entry = mSounds.GetFirst();
			do
			{
				nextEntry = mSounds.GetNext(entry);
				Release(entry->mVal);
			}
			while ( (entry = nextEntry) );
		}

		mSounds.Release();
	}
	g_audioLibrary.Unlock();
}


//============================================================================================================
// Sets the listener position, orientation and velocity
//============================================================================================================

void Audio::SetListener(const Vector3f& position, const Vector3f& direction, const Vector3f& up, const Vector3f& velocity)
{
	alListenerfv(AL_VELOCITY, velocity);
	ASSERT(alGetError() == 0, "AL error occured");

	alListenerfv(AL_POSITION, position);
	ASSERT(alGetError() == 0, "AL error occured");

	float orientation[] = { direction.x, direction.y, direction.z, up.x, up.y, up.z };
	alListenerfv(AL_ORIENTATION, orientation);
	ASSERT(alGetError() == 0, "AL error occured");	
}

//============================================================================================================
// Either returns a sound that's already in the library 
// or loads one into the library and returns it
//============================================================================================================

ISound* Audio::GetSound(const String& name, bool createIfMissing)
{
	ISound* retVal = NULL;

	g_audioLibrary.Lock();
	{
		// Check if the sound is already in the library

		if (mSounds.IsValid())
		{
			LinkedList<Sound*>::Entry* entry = mSounds.GetFirst();
			do
			{
				if(entry->mVal->GetName() == name)
				{
					return entry->mVal;
				}
			}
			while (entry = mSounds.GetNext(entry));
		}

		// If the sound isn't in the library, try to add it to the library
	
		if (createIfMissing)
		{
			AudioData *audioData = new AudioData;
			if (audioData->Load(name))
			{
				if (audioData->GetLength() > 150000)
				{
					StreamedSound *s = new StreamedSound(name, audioData);
					s->mSoundsEntry = mSounds.GetUnused();
					mSounds.Expand() = s;
					retVal = s;
				}
				else
				{
					StaticSound *s = new StaticSound(name, audioData);
					s->mSoundsEntry = mSounds.GetUnused();
					mSounds.Expand() = s;
					retVal = s;
				}
			}
		}
	}
	g_audioLibrary.Unlock();

	return retVal;
}

//============================================================================================================
// Deletes a Sound object from the library
//============================================================================================================

bool Audio::Release(ISound* sound)
{
	if (sound && ((Sound*)sound)->mRefCount == 0)
	{ 
		mSounds.Recycle(((Sound*)sound)->mSoundsEntry);
		delete sound;

		return true;
	}

	return false;
}

//============================================================================================================
// Creates a 3D sound instance at a specified position
//============================================================================================================

ISoundInstance* Audio::Instantiate(ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat)
{
	ASSERT(sound != NULL, "A NULL pointer has been passed to Audio::Instantiate() when creating a 3D sound instance");

	SoundInstance *instance = _Instantiate((Sound*)sound, layer, repeat);
	instance->SetPosition(position);

	return instance;
}

//============================================================================================================
// Creates a 2D sound instance
//============================================================================================================

ISoundInstance* Audio::Instantiate(ISound* sound, uint layer, float fadeInTime, bool repeat)
{
	ASSERT(sound != NULL, "A NULL pointer has been passed to Audio::Instantiate() when creating a 2D sound instance");

	SoundInstance *instance = _Instantiate((Sound*)sound, layer, repeat);
	instance->SetSpatial(false);

	return instance;
}

//============================================================================================================
// Updates all active sound instances
//============================================================================================================

void Audio::Update()
{
	g_audioLibrary.Lock();
	{
		if (mActiveInstances.IsValid())
		{
			LinkedList<SoundInstance*>::Entry* nextEntry;
			LinkedList<SoundInstance*>::Entry* instanceEntry = mActiveInstances.GetFirst();
			do
			{
				nextEntry = mActiveInstances.GetNext(instanceEntry);
				if (!instanceEntry->mVal->Update())
				{
					mActiveInstances.Recycle(instanceEntry);
				}
			}
			while ( (instanceEntry = nextEntry) );
		}
	}
	g_audioLibrary.Unlock();
}

//============================================================================================================
// INTERNAL
//============================================================================================================

SoundInstance* Audio::_Instantiate(Sound* sound, uint layer, bool repeat)
{
	SoundInstance *instance = sound->Instantiate();

	instance->SetRepeat(repeat);
	instance->mLayer = &mLayers[layer];
	instance->mActiveInstances = &mActiveInstances;	
	instance->SetVolume(instance->mVolume);
	instance->mInstancesEntry = mLayers[layer].mInstances.GetUnused();
	mLayers[layer].mInstances.Expand() = instance;

	return instance;
}

