#include "../Include/_All.h"
#include <vorbis/vorbisfile.h>
//#include <IrrKlang/Include/irrKlang.h>

//============================================================================================================
// Audio Library
//============================================================================================================

#ifdef _WINDOWS
//  #pragma comment(lib, "irrKlang.lib")
#endif

using namespace R5;

//============================================================================================================
// In order to abstract cAudio and make it invisible to the outside projects we keep it as a void*
//============================================================================================================

//#define IRRKLANG ((irrklang::ISoundEngine*)mAudioLib)
//#define SOURCE(source) ((irrklang::ISoundSource*)source)
#define SOUND(sound) ((Sound*)sound)

//============================================================================================================
// Initialize the cAudio library
//============================================================================================================

Audio::Audio() : mAudioLib(0), mPos (0.0f, 0.0f, 0.0f)
{
//	mAudioLib = irrklang::createIrrKlangDevice();
//	ASSERT(mAudioLib != 0, "Failed to start the Audio engine!\n");
	ALCdevice *device = alcOpenDevice(NULL);
	ASSERT(device, "Couldn't open default OpenAL device");
	
	ALCcontext *context = alcCreateContext(device, NULL);
	ASSERT(context, "Couldn't create default context");

	ALCboolean retval = alcMakeContextCurrent(context);
	ASSERT(retval, "Couldn't make the context current");
	
	alDistanceModel(AL_NONE);
	ASSERT(alGetError() == AL_NO_ERROR, "error");
}

//============================================================================================================

Audio::~Audio()
{
	Release();
	ALCcontext *context = alcGetCurrentContext();
	ALCdevice *device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
//	IRRKLANG->drop();
}

//============================================================================================================
// Release all audio resources
//============================================================================================================

void Audio::Release()
{
	Lock();
	{
//		IRRKLANG->stopAllSounds();
		mLayers.Release();
		mLibrary.Release();
	}
	Unlock();
}

//============================================================================================================
// Update the different sounds every frame.
//============================================================================================================

void Audio::Update()
{
	Lock();
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
	Unlock();
}

//============================================================================================================
// Release all resources associated with the specified sound
//============================================================================================================

void Audio::Release (ISound* sound)
{
	Lock();
	{
		if (sound != 0) _Release(sound);
	}
	Unlock();
}

//============================================================================================================
// Sets the sound listener pos/dir/up (usually should be the camera)
//============================================================================================================

void Audio::SetListener (const Vector3f& position, const Vector3f& dir, const Vector3f& up)
{
	mPos = position;
/*
	irrklang::vec3df p (position.x, position.z, position.y);
	irrklang::vec3df d (dir.x, dir.z, dir.y);
	irrklang::vec3df v (0.0f, 0.0f, 0.0f);
	irrklang::vec3df u (up.x, up.z, up.y);

	IRRKLANG->setListenerPosition(p, d, v, u);
*/
	alListenerfv(AL_POSITION, position);

	ALfloat orient[] = {dir.x, dir.y, dir.z, up.x, up.y, up.z};
	alListenerfv(AL_ORIENTATION, orient);
}

//============================================================================================================
// Set the layers volume
//============================================================================================================

void Audio::SetLayerVolume (uint layer, float volume, float duration)
{
	Lock();
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
	Unlock();
}

//============================================================================================================
// Get the volume of a layer if the layer exists
//============================================================================================================

const float Audio::GetLayerVolume (uint layer) const
{
	float volume (1.0f);
	AudioLayer* audioLayer = mLayers.GetIfExists(layer);
	if (audioLayer != 0) volume = audioLayer->mVolume;
	return volume;
}

//============================================================================================================
// Creates a AudioSource
//============================================================================================================

R5::ISound* Audio::GetSound (const String& name, bool createIfMissing)
{
	SoundPtr sound = mLibrary.Find(name);
	if (sound == 0 && createIfMissing)
	{
		Lock();
		{
			sound = mLibrary.AddUnique(name);
			sound->SetAudio(this);

			ALuint buffer;
			alGenBuffers(1, &buffer);

			//read the audio file
			FILE *f = fopen(name, "rb");
			OggVorbis_File ogg;
			ov_open_callbacks(f, &ogg, NULL, 0, OV_CALLBACKS_DEFAULT);

			vorbis_info *vorbisInfo;
			vorbisInfo = ov_info(&ogg, -1);

			ALenum format;
			if (vorbisInfo->channels == 1)
				format = AL_FORMAT_MONO16;
			else
				format = AL_FORMAT_STEREO16;

			Memory oggData;
			long bytesRead;
			int bitStream = 0;
			do
			{
				char data[4096];
				bytesRead = ov_read(&ogg, data, sizeof(data), 0, 2, 1, &bitStream);
				oggData.Append((void*)data, (uint)bytesRead);
			} while(bytesRead > 0);


			alBufferData(buffer, format, oggData.GetBuffer(), oggData.GetSize(), (ALsizei)vorbisInfo->rate);
//			sound->SetSource(IRRKLANG->addSoundSourceFromFile(name));
			sound->SetSource(buffer);
		}
		Unlock();
		
	}
	return sound;
}

//============================================================================================================
// Releases the cAudio Sound source.
//============================================================================================================

void Audio::ReleaseInstance(ISoundInstance* sound)
{
	Lock();
	{
		SoundInstance* inst = (SoundInstance*)sound;
		AudioLayer* audioLayer = _GetAudioLayer(inst->mLayer, 1.0f);
		audioLayer->mSounds.Remove(sound);
		delete sound;
	}
	Unlock();
}

//============================================================================================================
// Returns a newly created 2D sound source. Needs to return void* as irrklang is not defined outside this
// class and the SoundInstance class
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat)
{
	SoundInstance* soundInst = _Instantiate(SOUND(sound), layer, fadeInTime, repeat);
//	irrklang::ISound* soundval = IRRKLANG->play2D(SOURCE(sound->GetSource()), repeat, false, true, true);
//	soundInst->mAudioSource = soundval;
//	SOUND(soundval)->setVolume(0.0f);
	ALuint source;
	alGenSources(1, &source);
	alSourcei( source, AL_BUFFER, SOUND(sound)->GetBuffer() );
	alSourcei( source, AL_LOOPING, (ALuint)repeat);
	alSourcef( source, AL_GAIN, 0.0f );
	alSourcei( source, AL_SOURCE_RELATIVE, 1 );
	alSourcePlay(source);
	soundInst->mSource = source;
	return soundInst;
}

//============================================================================================================
// Returns a newly created 3D sound source.
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat)
{
	SoundInstance* soundInst = _Instantiate(SOUND(sound), layer, fadeInTime, repeat);
//	irrklang::vec3df pos (position.x, position.y, position.z);
//	irrklang::ISoundSource* soundSource = SOURCE(sound->GetSource());
//	irrklang::ISound* soundval = IRRKLANG->play3D(soundSource, pos, repeat, false, true, true);
//	soundInst->mAudioSource = soundval;
//	SOUND(soundval)->setMinDistance(1000.0f);
//	SOUND(soundval)->setMaxDistance(1000.0f);
//	SOUND(soundval)->setVolume(0.0f);
	ALuint source;
	alGenSources(1, &source);
	alSourcefv( source, AL_POSITION, position );
	alSourcei( source, AL_BUFFER, SOUND(sound)->GetBuffer() );
	alSourcei( source, AL_LOOPING, (ALuint)repeat);
	alSourcef( source, AL_GAIN,	0.0f );
	alSourcePlay(source);
	soundInst->mSource = source;
	soundInst->mIs3D = true;
	return soundInst;
}

//============================================================================================================
// INTERNAL: Release the specified sound
//============================================================================================================

void Audio::_Release (ISound* sound)
{
	if (sound != 0)
	{
		mLibrary.Delete(R5_CAST(Sound, sound));
	}
}

//============================================================================================================
// INTERNAL: Retrieves the specified audio layer
//============================================================================================================

Audio::AudioLayer* Audio::_GetAudioLayer (uint layer, float volume)
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
	// Load the data
	String name = sound->GetName();
	SoundInstance* soundInst = new SoundInstance();
	
	soundInst->mSound = sound;
	soundInst->mLayer = layer;
	soundInst->mIsPlaying = true;
	soundInst->mIsPaused = false;

	Lock();
	{
		AudioLayer* audioLayer = _GetAudioLayer(layer, 1.0f);
		PlayingSounds& playing = audioLayer->mSounds;
		soundInst->SetVolume(1.0f, fadeInTime);
		soundInst->SetRepeat(repeat);
		playing.Expand() = soundInst;
	}
	Unlock();
	return soundInst;
}
