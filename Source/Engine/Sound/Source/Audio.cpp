#include "../Include/_All.h"

//============================================================================================================
//  Audio Library
//============================================================================================================

#include <CAudio/Include/cAudio.h>

#ifdef _WINDOWS
	#pragma comment(lib, "cAudio.lib")
#endif

using namespace R5;
using namespace cAudio;

IAudioManager* mAudioManager;

//============================================================================================================

Audio::Audio()
{
	mAudioManager = createAudioManager();
}

Audio::~Audio()
{
	mAudioFiles.Release();

	//Shutdown cAudio
	mAudioManager->releaseAllSources();
	mAudioManager->shutDown();
	destroyAudioManager(mAudioManager);
}

//============================================================================================================
// Load the requested file and create a AudioSource for it
//============================================================================================================

IAudioSource* Load (String& fileName)
{
	// Temporary memory buffer used to load the file
	Memory in;
	
	if (mAudioManager != 0 && in.Load(fileName))
	{
		return mAudioManager->createFromMemory(fileName, (char*)in.GetBuffer(), in.GetSize(), System::GetExtensionFromFilename(fileName));
	}
	return 0;
}

//============================================================================================================
// Create a AudioSource
//============================================================================================================

IAudio::Sound* Audio::CreateAudioSource (String& fileName)
{
	Sound* sound			= new Sound();
	sound->mFileName		= fileName;
	mAudioFiles[fileName]	= sound;
	return sound;
}

//============================================================================================================
// Update the different sounds every frame.
//============================================================================================================

void Audio::Update()
{
	ulong currentTime = Time::GetMilliseconds();

	PointerArray<Sound>& values = mAudioFiles.GetAllValues();
	
	for (uint b = values.GetSize(); b > 0;)
	{
		Sound* sound = values[--b];
		IAudioSource* source = ((IAudioSource*)sound->mAudioSource);
		
		if (source != 0 && source->isPlaying())
		{
			// Volume has changed fade to new volume
			if (sound->mVolume.y != sound->mVolume.z)
			{
				float factor = (sound->mDuration > 0) ? (float)(0.001 * (currentTime - sound->mStart)) / (float)sound->mDuration  : 1.0f;
				sound->mVolume.y = Interpolation::Linear(sound->mVolume.x, sound->mVolume.z, factor);
				source->setVolume(sound->mVolume.y);

				// When the sound has fully fade perform event
				if(sound->mVolume.y <= 0.0f) 
				{
					if(sound->mEvent == Event::STOP)
					{
						source->stop();
					}
					else if (sound->mEvent == Event::PAUSE)
					{
						source->pause();
					}
				}
			}
		}
	}
}

//============================================================================================================
// Plays a 2D sound
//============================================================================================================

void Audio::Play (String& fileName, uint layer, bool loop, float volume, float duration)
{
	PlaySound(fileName, layer, loop, volume, duration);

	Sound* sound = mAudioFiles.GetIfExists(fileName);
	
	if (sound != 0 && sound->mAudioSource != 0)
	{
		IAudioSource* source = ((IAudioSource*)sound->mAudioSource);

		if (source != 0)
		{
			source->play2d(loop);
		}
	}
}

//============================================================================================================
// Plays a 3D sound
//============================================================================================================

void Audio::Play(String& fileName, Vector3f position, uint layer, bool loop, float volume, float duration)
{
	PlaySound(fileName, layer, loop, volume, duration);

	Sound* sound = mAudioFiles.GetIfExists(fileName);
	
	if (sound != 0 && sound->mAudioSource != 0)
	{
		SetPosition	(sound, position);

		IAudioSource* source = ((IAudioSource*)sound->mAudioSource);

		if (source != 0)
		{
			cVector3 pos (position.x, position.y, position.z);
			source->play3d(pos, 1.0f, loop);
		}
	}
}

//============================================================================================================
// Helper function to separates common functionality between 2D and 3D sound
//============================================================================================================

void Audio::PlaySound (String& fileName, uint layer, bool loop, float volume, float duration)
{
	AudioLayer* audioLayer = mAudioLayers.GetIfExists(layer);
	
	// If the audio layer isn't found create one.
	if (audioLayer == 0)
	{
		AudioLayer* al = new AudioLayer(layer,volume);
		mAudioLayers[layer] = al;
		audioLayer = mAudioLayers.GetIfExists(layer);
	}

	PlayingSounds& sounds = audioLayer->mPlayingSound;
	
	// For all the playing sounds on this layer fade them out.
	if (sounds.IsValid())
	{
		for (uint b = sounds.GetSize(); b > 0; )
		{
			Sound* sound = mAudioFiles[sounds[--b]->mFileName];

			if(sounds[b]->mFileName != fileName)
			{
				Stop(sound);
			}
		}
	}
	
	Sound* sound = mAudioFiles.GetIfExists(fileName);

	// If the sound hasn't been created create it and set default parameters.
	if (sound == 0)
	{
		sound = CreateAudioSource(fileName);
		sounds.Expand()	= sound;
	}

	if (sound->mAudioSource == 0)
	{
		sound->mAudioSource = Load(fileName);
	}

	// If the AudioSource is valid play the sound.
	if (sound->mAudioSource != 0)
	{
		sound->mLayer = layer;
		SetVolume	(sound, volume);
		SetLooping	(sound, loop);		
	}
}

//============================================================================================================
// Stop the audio by fading it out over duration
//============================================================================================================

void Audio::Stop (String& fileName, float duration)
{
	Stop(mAudioFiles.GetIfExists(fileName), duration);	
}

void Audio::Stop (Sound* sound, float duration)
{
	if (sound != 0 && sound->mVolume.z > 0.0f)
	{
		sound->mEvent = Event::STOP;
		SetVolume(sound, 0.0f, duration);
	}
}

//============================================================================================================
// Pause the audio by fading it out over duration
//============================================================================================================

void Audio::Pause (String& fileName, float duration)
{
	Pause(mAudioFiles.GetIfExists(fileName), duration);	
}

void Audio::Pause (Sound* sound, float duration)
{
	if (sound != 0 && sound->mVolume.z > 0.0f)
	{
		sound->mEvent = Event::PAUSE;
		SetVolume(sound, 0.0f, duration);
	}
}

//============================================================================================================
// Set the layers volume
//============================================================================================================

void Audio::SetLayerVolume(uint layer, float volume, float duration)
{
	AudioLayer* audioLayer = mAudioLayers.GetIfExists(layer);
	
	// If the audio layer isn't found create one.
	if (audioLayer == 0)
	{
		AudioLayer* al = new AudioLayer(layer,volume);
		mAudioLayers[layer] = al;
		audioLayer = mAudioLayers.GetIfExists(layer);
	}

	audioLayer->mVolume = volume;

	PlayingSounds& sounds = audioLayer->mPlayingSound;
	
	// For all the playing sounds update there volume level
	if (sounds.IsValid())
	{
		for (uint b = sounds.GetSize(); b > 0; )
		{
			Sound* sound = mAudioFiles[sounds[--b]->mFileName];
			SetVolume(sound, sound->mVolume.y, duration);
		}
	}
}

//============================================================================================================
// Set the volume of a sound
//============================================================================================================

void Audio::SetSoundVolume(String& fileName, float volume, float duration)
{
	Sound* sound = mAudioFiles.GetIfExists(fileName);
	
	if (sound == 0)
	{
		sound = CreateAudioSource(fileName);
	}
	SetVolume(sound, volume, duration);	
}

//============================================================================================================
// Helper function to set the volume for the sound base on the layers volume as well
//============================================================================================================

void Audio::SetVolume(Sound* sound, float volume, float duration)
{
	if (sound != 0)
	{
		AudioLayer* audioLayer = mAudioLayers.GetIfExists(sound->mLayer);

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
// Get the volume of a layer if the layer exists
//============================================================================================================

const float Audio::GetLayerVolume (uint layer) const
{
	AudioLayer* audioLayer = mAudioLayers.GetIfExists(layer);
	
	if (audioLayer != 0)
	{
		return audioLayer->mVolume;
	}
	return 0.0f;
}

//============================================================================================================
// Get the volume of a sound if it exists
//============================================================================================================

const float Audio::GetSoundVolume (String& fileName) const
{
	Sound* sound = mAudioFiles.GetIfExists(fileName);

	if (sound != 0)
	{
		return sound->mVolume.y;
	}
	return 0.0f;
}

//============================================================================================================
// Sets the position a sound source
//============================================================================================================

void Audio::SetSoundPosition (String& fileName, Vector3f position)
{
	Sound* sound = mAudioFiles.GetIfExists(fileName);

	if (sound == 0)
	{
		sound = CreateAudioSource(fileName);
	}
	SetPosition(sound, position);
}

//============================================================================================================
// Helper function to set position
//============================================================================================================

void Audio::SetPosition (Sound* sound, Vector3f position)
{
	if (sound != 0)
	{
		sound->mPosition = position;

		if (sound->mAudioSource != 0)
		{
			IAudioSource* source = ((IAudioSource*)sound->mAudioSource);
			cVector3 pos (position.x, position.y, position.z);
			source->setPosition(pos);
		}
	}
}

//============================================================================================================
// Sets the position of the listener
//============================================================================================================

void Audio::SetListenerPosition (const Vector3f position)
{
	if (mAudioManager != 0)
	{
		IListener* listener = mAudioManager->getListener();
		cVector3 pos (position.x, position.y, position.z);
		listener->setPosition(pos);
	}
}

//============================================================================================================
// Sets the sound source is looping
//============================================================================================================

void Audio::SetSoundLooping (String& fileName, bool loop)
{
	Sound* sound = mAudioFiles.GetIfExists(fileName);

	if (sound == 0)
	{
		sound = CreateAudioSource(fileName);
	}
	SetLooping(sound, loop);
}

//============================================================================================================
// Helper function to set looping
//============================================================================================================

void Audio::SetLooping (Sound* sound, bool loop)
{
	if (sound != 0)
	{
		sound->mLoop = loop;

		if (sound->mAudioSource != 0)
		{
			((IAudioSource*)sound->mAudioSource)->loop(loop);
		}
	}
}