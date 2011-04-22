#include "../Include/_All.h"
#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"
using namespace R5;

AudioLibrary g_audioLibrary;

//============================================================================================================
// Initialize the Audio library
//============================================================================================================

Audio::Audio()
{
	ALCdevice* device = alcOpenDevice(NULL);
	ASSERT(device != 0, "Couldn't open the default OpenAL device");
	
	ALCcontext* context = alcCreateContext(device, NULL);
	ASSERT(context != 0, "Couldn't create the default context");

	ALCboolean retval = alcMakeContextCurrent(context);
	ASSERT(retval != 0, "Couldn't make the context current");
	
	alDistanceModel(AL_NONE);
	ASSERT(alGetError() == AL_NO_ERROR, "error");
}

//============================================================================================================

Audio::~Audio()
{
	Release();
	ALCcontext* context = alcGetCurrentContext();
	ALCdevice* device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

//============================================================================================================
// Release all audio resources
//============================================================================================================

void Audio::Release()
{
	g_audioLibrary.Lock();
	{
		mLayers.Release();
		mLibrary.Release();
	}
	g_audioLibrary.Unlock();
}

//============================================================================================================
// Update the different sounds every frame.
//============================================================================================================

void Audio::Update()
{
	g_audioLibrary.Lock();
	{
		ulong currentTime = Time::GetMilliseconds();
		PointerArray<AudioLayer>& values = mLayers.GetAllValues();

		FOREACH(b, values)
		{
			AudioLayer* audioLayer = values[b];

			FOREACH(j, audioLayer->mSounds)
			{
				SoundInstance* sound = (SoundInstance*)audioLayer->mSounds[j];
				sound->Update(currentTime);
			}
		}
	}
	g_audioLibrary.Unlock();
}

//============================================================================================================
// Release all resources associated with the specified sound
//============================================================================================================

void Audio::Release (ISound* sound)
{
	g_audioLibrary.Lock();
	if (sound != 0) mLibrary.Delete((Sound*)sound);
	g_audioLibrary.Unlock();
}

//============================================================================================================
// Sets the sound listener pos/dir/up (usually should be the camera)
//============================================================================================================

void Audio::SetListener (const Vector3f& position, const Vector3f& dir, const Vector3f& up)
{
	mPos = position;
	alListenerfv(AL_POSITION, position);

	ALfloat orient[] = {dir.x, dir.y, dir.z, up.x, up.y, up.z};
	alListenerfv(AL_ORIENTATION, orient);
}

//============================================================================================================
// Set the layers volume
//============================================================================================================

void Audio::SetLayerVolume (uint layer, float volume, float duration)
{
	g_audioLibrary.Lock();
	{
		AudioLayer* audioLayer = mLayers.GetIfExists(layer);
		
		// If the audio layer isn't found create one.
		if (audioLayer == 0)
		{
			AudioLayer* al = new AudioLayer(layer,volume);
			mLayers[layer] = al;
			audioLayer = mLayers.GetIfExists(layer);
		}

		audioLayer->mVolume = volume;

		PlayingSounds& sounds = audioLayer->mSounds;
		
		// For all the playing sounds update there volume level
		if (sounds.IsValid())
		{
			for (uint b = sounds.GetSize(); b > 0; )
			{
				ISoundInstance* sound = sounds[--b];
				sound->SetVolume(sound->GetVolume(), duration);
			}
		}
	}
	g_audioLibrary.Unlock();
}

//============================================================================================================
// Gets the volume of the specified layer
//============================================================================================================

const float Audio::GetLayerVolume (uint layer) const
{
	float volume (1.0f);
	AudioLayer* audioLayer = mLayers.GetIfExists(layer);
	if (audioLayer != 0) volume = audioLayer->mVolume;
	return volume;
}

//============================================================================================================
// Adds a new sound to the library
//============================================================================================================

R5::ISound* Audio::GetSound (const String& name, bool createIfMissing)
{
	Sound* sound (0);
	g_audioLibrary.Lock();
	sound = _GetOrCreate(name, createIfMissing);
	g_audioLibrary.Unlock();
	return sound;
}

//============================================================================================================
// Release the specified sound instance
//============================================================================================================

void Audio::ReleaseInstance(ISoundInstance* sound)
{
	g_audioLibrary.Lock();
	{
		SoundInstance* inst = (SoundInstance*)sound;
		AudioLayer* audioLayer = _GetAudioLayer(inst->mLayer, 1.0f);
		audioLayer->mSounds.Remove(sound);
		delete sound;
	}
	g_audioLibrary.Unlock();
}

//============================================================================================================
// Create a new instance of the specified 2D sound
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat)
{
	if (sound != 0)
	{
		SoundInstance* soundInst = _Instantiate((Sound*)sound, layer, fadeInTime, repeat);

		if (sound->IsValid())
		{
			uint source;
			alGenSources(1, &source);
			alSourcei( source, AL_BUFFER, ((Sound*)sound)->mBuffer );
			alSourcei( source, AL_LOOPING, (uint)repeat);
			alSourcef( source, AL_GAIN, 0.0f );
			alSourcei( source, AL_SOURCE_RELATIVE, 1 );
			alSourcePlay(source);
			soundInst->mSource = source;
		}
		return soundInst;
	}
	return 0;
}

//============================================================================================================
// Create a new instance of the specified 3D sound
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat)
{
	if (sound != 0)
	{
		SoundInstance* soundInst = _Instantiate((Sound*)sound, layer, fadeInTime, repeat);

		if (sound->IsValid())
		{
			uint source;
			alGenSources(1, &source);
			alSourcefv( source, AL_POSITION, position );
			alSourcei( source, AL_BUFFER, ((Sound*)sound)->mBuffer );
			alSourcei( source, AL_LOOPING, (uint)repeat);
			alSourcef( source, AL_GAIN,	0.0f );
			alSourcePlay(source);
			soundInst->mSource = source;
		}

		soundInst->mIs3D = true;
		return soundInst;
	}
	return 0;
}

//============================================================================================================
// INTERNAL: Gets or creates the specified sound
//============================================================================================================

inline Sound* Audio::_GetOrCreate (const String& name, bool createIfMissing)
{
	FOREACH(i, mLibrary)
	{
		Sound* s = mLibrary[i];
		if (s != 0 && s->GetName() == name) return s;
	}

	if (createIfMissing)
	{
		Sound* s = new Sound(this, name);
		mLibrary.Expand() = s;
		return s;
	}
	return 0;
}

//============================================================================================================
// INTERNAL: Retrieves the specified audio layer
//============================================================================================================

inline Audio::AudioLayer* Audio::_GetAudioLayer (uint layer, float volume)
{
	AudioLayerPtr& ptr = mLayers[layer];
	if (ptr == 0) ptr = new AudioLayer(layer, volume);
	return ptr;
}

//============================================================================================================
// INTERNAL: Set common properties for a sound
//============================================================================================================

SoundInstance* Audio::_Instantiate (Sound* sound, uint layer, float fadeInTime, bool repeat)
{
	String name = sound->GetName();
	SoundInstance* soundInst = new SoundInstance();
	
	soundInst->mSound = sound;
	soundInst->mLayer = layer;
	soundInst->mIsPlaying = true;
	soundInst->mIsPaused = false;

	g_audioLibrary.Lock();
	{
		AudioLayer* audioLayer = _GetAudioLayer(layer, 1.0f);
		PlayingSounds& playing = audioLayer->mSounds;
		soundInst->SetVolume(1.0f, fadeInTime);
		soundInst->SetRepeat(repeat);
		playing.Expand() = soundInst;
	}
	g_audioLibrary.Unlock();
	return soundInst;
}
