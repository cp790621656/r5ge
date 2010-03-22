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

#define SOURCE(source) ((cAudio::IAudioSource*)source)

//============================================================================================================
// Check to see if the sound is playing
//============================================================================================================

const bool SoundInstance::IsPlaying()	const
{
	bool isPlaying = false;

	if (mAudioSource != 0)
	{
		isPlaying = SOURCE(mAudioSource)->isPlaying();
	}
	return isPlaying;
}

//============================================================================================================
// Check to see if the sound is paused
//============================================================================================================

const bool SoundInstance::IsPaused()	const
{
	bool isPaused = false;

	if (mAudioSource != 0)
	{
		isPaused = SOURCE(mAudioSource)->isPaused();
	}
	return isPaused;
}

//============================================================================================================
// Destoryes the sound instance releasing the memory
//============================================================================================================

void SoundInstance::DestroySelf()
{
	((Audio*)mSound->GetAudio())->ReleaseInstance(this);
}

//============================================================================================================
// Play the sound
//============================================================================================================

void SoundInstance::Play()
{
	if (mAudioSource !=0)
	{
		if (IsPaused())
		{
			SOURCE(mAudioSource)->play();
			SetVolume(mVolume.w, 0.0f);
		}
		else if (!mIs3D)
		{
			SOURCE(mAudioSource)->play2d(mRepeat);
		}
		else
		{
			cAudio::cVector3 pos (mPosition.x, mPosition.y, mPosition.z);
			SOURCE(mAudioSource)->play3d(pos, 2.0f, mRepeat);
			SOURCE(mAudioSource)->setMinDistance(1000.0f);
			SOURCE(mAudioSource)->setMaxDistance(1000.0f);
		}
	}
}

//============================================================================================================
// Pause the sound
//============================================================================================================

void SoundInstance::Pause (float duration)
{
	if (IsPlaying())
	{
		if (mVolume.z > 0.0f)
		{
			mAction = TargetAction::Pause;
			_SetVolume(mVolume.w, 0.0f, duration);
		}
	}
}

//============================================================================================================
// Stop the sound playback
//============================================================================================================

void SoundInstance::Stop (float duration)
{
	if (IsPlaying())
	{
		if (mVolume.z > 0.0f)
		{
			mAction = TargetAction::Stop;
			SetVolume (0.0f, duration);
		}
	}
}

//============================================================================================================
// Sets the 3D position of the specified sound
//============================================================================================================

void SoundInstance::SetPosition (const Vector3f& position)
{
	mPosition = position;

	if (mAudioSource != 0)
	{
		cAudio::cVector3 pos (position.x, position.y, position.z);
		SOURCE(mAudioSource)->setPosition (pos);
	}
}

//============================================================================================================
// Changes the volume of the specified sound
//============================================================================================================

void SoundInstance::SetVolume (float volume, float duration)
{
	float layerVolume = mSound->GetAudio()->GetLayerVolume(mLayer);
	
	_SetVolume (volume, layerVolume * volume, duration);
}

//============================================================================================================
// Sets whether the sound will repeat after it ends
//============================================================================================================

void SoundInstance::SetRepeat (bool repeat)
{
	mRepeat = repeat;

	if (mAudioSource != 0)
	{
		SOURCE(mAudioSource)->loop(repeat);
	}
}

//============================================================================================================
// INTERNAL: Set the volume of the sound
//============================================================================================================

void SoundInstance::_SetVolume (float volume, float calculatedVolume, float duration)
{
	mVolume.x = mVolume.y;
	mVolume.z = calculatedVolume;
	mVolume.w = volume;
	mStart = Time::GetMilliseconds();
	mDuration = duration;
}