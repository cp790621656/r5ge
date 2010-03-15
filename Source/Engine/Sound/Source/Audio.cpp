#include "../Include/_All.h"

//============================================================================================================
// Audio Library
//============================================================================================================

#include <CAudio/Include/cAudio.h>

#ifdef _WINDOWS
  #pragma comment(lib, "cAudio.lib")
#endif

using namespace R5;

//============================================================================================================
// In order to abstract cAudio and make it invisible to the outside projects we keep it as a void*
//============================================================================================================

#define CAUDIO ((cAudio::IAudioManager*)mAudioLib)
#define SOURCE(source) ((cAudio::IAudioSource*)source)

//============================================================================================================
// Initialize the cAudio library
//============================================================================================================

Audio::Audio() : mAudioLib(0)
{
	// cAudio has a strange habit of leaving an HTML log file by default. This disables it.
	cAudio::ILogger* logger = cAudio::getLogger();
	logger->unRegisterLogReceiver("File");

	// Initialize the cAudio library
	mAudioLib = cAudio::createAudioManager();
}

//============================================================================================================

Audio::~Audio()
{
	Release();
	CAUDIO->shutDown();
	cAudio::destroyAudioManager(CAUDIO);
}

//============================================================================================================
// Release all audio resources
//============================================================================================================

void Audio::Release()
{
	Lock();
	{
		mLayers.Release();
		mLibrary.Release();
		CAUDIO->releaseAllSources();
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
		PointerArray<Sound>& values = mLibrary.GetAllValues();

		for (uint b = values.GetSize(); b > 0;)
		{
			Sound* sound = values[--b];
			cAudio::IAudioSource* source = SOURCE(sound->mAudioSource);

			// Only continue if the sound is actually playing
			if (source != 0 && source->isPlaying())
			{
				// If the sound volume is changing, we need to adjust it inside cAudio
				if (sound->mVolume.y != sound->mVolume.z)
				{
					// Calculate the current fade-out factor
					float factor = (sound->mDuration > 0) ? (float)(0.001 * (currentTime - sound->mStart)) /
						sound->mDuration : 1.0f;

					if (factor < 1.0f)
					{
						// Update the volume
						sound->mVolume.y = Interpolation::Linear(sound->mVolume.x, sound->mVolume.z, factor);
						source->setVolume(sound->mVolume.y);
					}
					else
					{
						// We've reached the end of the fading process
						sound->mVolume.y = sound->mVolume.z;
						source->setVolume(sound->mVolume.y);

						// If the sound was fading out, perform the target action
						if (sound->mVolume.y < 0.0001f)
						{
							if (sound->mAction == TargetAction::Stop)
							{
								source->stop();
							}
							else if (sound->mAction == TargetAction::Pause)
							{
								source->pause();
							}
						}
					}
				}
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Adds a new sound to the library
//============================================================================================================

void Audio::Add (const String& name, const byte* buffer, uint size)
{
	// Make sure to release any existing data
	Release(name);

	Lock();
	{
		// Release any previous data
		Sound* sound = mLibrary.GetIfExists(name);
		if (sound != 0) _Release(sound);

		// Add a new sound
		sound = _AddSound(name);

		// Load the data
		sound->mAudioSource = CAUDIO->createFromMemory(name, (char*)buffer, size,
			System::GetExtensionFromFilename(name).GetBuffer());
	}
	Unlock();
}

//============================================================================================================
// Plays a 2D sound
//============================================================================================================

void Audio::Play (const String& name, uint layer, bool repeat, float volume, float duration)
{
	Lock();
	{
		Sound* sound = _PlaySound(name, layer, repeat, volume, duration);

		if (sound != 0 && sound->mAudioSource != 0)
		{
			SOURCE(sound->mAudioSource)->play2d(repeat);
		}
	}
	Unlock();
}

//============================================================================================================
// Plays a 3D sound
//============================================================================================================

void Audio::Play (const String& name, const Vector3f& position, uint layer, bool repeat, float volume, float duration)
{
	Lock();
	{
		Sound* sound = _PlaySound(name, layer, repeat, volume, duration);
		
		if (sound != 0 && sound->mAudioSource != 0)
		{
			_SetPosition (sound, position);

			cAudio::cVector3 pos (position.x, position.y, position.z);
			SOURCE(sound->mAudioSource)->play3d(pos, 1.0f, repeat);
		}
	}
	Unlock();
}

//============================================================================================================
// Pause the sound by fading it out over the specified time duration
//============================================================================================================

void Audio::Pause (const String& name, float duration)
{
	Lock();
	{
		Sound* sound = mLibrary.GetIfExists(name);
		if (sound != 0) _Pause(sound, duration);	
	}
	Unlock();
}

//============================================================================================================
// Stop the sound by fading it out over the specified time duration
//============================================================================================================

void Audio::Stop (const String& name, float duration)
{
	Lock();
	{
		Sound* sound = mLibrary.GetIfExists(name);
		if (sound != 0) _Stop(sound, duration);	
	}
	Unlock();
}

//============================================================================================================
// Release all resources associated with the specified sound
//============================================================================================================

void Audio::Release (const String& name)
{
	Lock();
	{
		Sound* sound = mLibrary.GetIfExists(name);
		if (sound != 0) _Release(sound);
	}
	Unlock();
}

//============================================================================================================
// Sets the sound listener position (usually should be the camera's position)
//============================================================================================================

void Audio::SetListenerPosition (const Vector3f& position)
{
	cAudio::cVector3 pos (position.x, position.y, position.z);
	CAUDIO->getListener()->setPosition(pos);
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
				Sound* sound = mLibrary[sounds[--b]->mName];
				_SetVolume(sound, sound->mVolume.y, duration);
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
	float volume (0.0f);
	Lock();
	{
		AudioLayer* audioLayer = mLayers.GetIfExists(layer);
		if (audioLayer != 0) volume = audioLayer->mVolume;
	}
	Unlock();
	return volume;
}

//============================================================================================================
// Get the volume of a sound if it exists
//============================================================================================================

const float Audio::GetSoundVolume (const String& name) const
{
	float volume (0.0f);
	Lock();
	{
		Sound* sound = mLibrary.GetIfExists(name);
		if (sound != 0) volume = sound->mVolume.y;
	}
	Unlock();
	return volume;
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
// INTERNAL: Create a AudioSource
//============================================================================================================

Audio::Sound* Audio::_AddSound (const String& name)
{
	SoundPtr& sound = mLibrary[name];
	if (sound == 0) sound = new Sound(name);
	return sound;
}

//============================================================================================================
// INTERNAL: Play the specified sound
//============================================================================================================

Audio::Sound* Audio::_PlaySound (const String& name, uint layer, bool loop, float volume, float duration)
{
	AudioLayer* audioLayer = _GetAudioLayer(layer, volume);

	PlayingSounds& playing = audioLayer->mSounds;

	Sound* sound = 0;
	bool isPlaying = false;

	// Fade out all the sounds playing on the same layer
	if (playing.IsValid())
	{
		for (uint b = playing.GetSize(); b > 0; )
		{
			Sound* current = mLibrary[playing[--b]->mName];

			if (current->mName == name)
			{
				sound = current;
				isPlaying = true;
			}
			else
			{
				_Stop(current, duration);
			}
		}
	}

	// Create the sound if it hasn't been found
	if (sound == 0) sound = _AddSound(name);

	// If the sound hasn't been loaded, try to load it
	if (sound->mAudioSource == 0)
	{
		Memory mem;

		if (mem.Load(name.GetBuffer()))
		{
			sound->mAudioSource = CAUDIO->createFromMemory(name, (char*)mem.GetBuffer(), mem.GetSize(),
				System::GetExtensionFromFilename(name).GetBuffer());
		}
	}

	// If the AudioSource is valid, play the sound
	if (sound->mAudioSource != 0)
	{
		sound->mLayer = layer;

		_SetVolume(sound, volume);
		_SetRepeat(sound, loop);

		if (!isPlaying) playing.Expand() = sound;
		return sound;
	}

	// No audio source -- the sound is not valid
	WARNING("Trying to play a sound that has not been loaded");
	return 0;
}

//============================================================================================================
// INTERNAL: Release the specified sound
//============================================================================================================

void Audio::_Release (Sound* sound)
{
	if (sound != 0)
	{
		PointerArray<AudioLayer>& layers = mLayers.GetAllValues();

		for (uint i = 0; i < layers.GetSize(); ++i)
		{
			AudioLayer* layer = layers[i];
			layer->mSounds.Remove(sound);
		}

		if (sound->mAudioSource != 0)
		{
			CAUDIO->release( SOURCE(sound->mAudioSource) );
			sound->mAudioSource = 0;
		}
		mLibrary.Delete(sound->mName);
	}
}

//============================================================================================================
// INTERNAL: Pause the playback of the specified sound temporarily, fading it out over the specified duration
//============================================================================================================

void Audio::_Pause (Sound* sound, float duration)
{
	if (sound != 0 && sound->mVolume.z > 0.0f)
	{
		sound->mAction = TargetAction::Pause;
		_SetVolume(sound, 0.0f, duration);
	}
}

//============================================================================================================
// INTERNAL: Stop playing the specified sound, fading it out over the specified duration
//============================================================================================================

void Audio::_Stop (Sound* sound, float duration)
{
	if (sound != 0 && sound->mVolume.z > 0.0f)
	{
		sound->mAction = TargetAction::Stop;
		_SetVolume(sound, 0.0f, duration);
	}
}

//============================================================================================================
// INTERNAL: Changes the volume of the specified sound
//============================================================================================================

void Audio::_SetVolume (Sound* sound, float volume, float duration)
{
	if (sound != 0)
	{
		AudioLayer* audioLayer = mLayers.GetIfExists(sound->mLayer);

		if (audioLayer != 0)
		{
			sound->mVolume.x = sound->mVolume.y;
			sound->mVolume.z = audioLayer->mVolume * volume;
			sound->mStart = Time::GetMilliseconds();
			sound->mDuration = duration;
		}
	}
}

//============================================================================================================
// INTERNAL: Sets whether the sound will repeat after it ends
//============================================================================================================

void Audio::_SetRepeat (Sound* sound, bool repeat)
{
	if (sound != 0)
	{
		if (sound->mAudioSource != 0)
		{
			SOURCE(sound->mAudioSource)->loop(repeat);
		}
	}
}

//============================================================================================================
// INTERNAL: Sets the 3D position of the specified sound
//============================================================================================================

void Audio::_SetPosition (Sound* sound, Vector3f position)
{
	if (sound != 0)
	{
		sound->mPosition = position;

		if (sound->mAudioSource != 0)
		{
			cAudio::cVector3 pos (position.x, position.y, position.z);
			SOURCE(sound->mAudioSource)->setPosition(pos);
		}
	}
}