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
// Update the sound
//============================================================================================================

void SoundInstance::Update(ulong time)
{
	if (!mIsPaused)
	{
		mDuration += Time::GetDeltaMS();
	}

	float atten = 1.0f;
		
	if (mIs3D)
	{
		atten = 1.0f - Float::Min((mPosition - mSound->GetAudio()->GetListener()).Magnitude() / 
			(mRange.y - mRange.x), 1.0f);
		atten *= atten;

		if (atten < 0.0001f)
		{
			if (mAudioSource != 0)
			{
				SOURCE(mAudioSource)->stop();
				mSound->GetAudio()->ReleaseAudioSource(mAudioSource);
				mAudioSource = 0;
			}
		}
		else if (mAudioSource == 0)
		{
			mAudioSource = mSound->GetAudio()->CreateAudioSource(mSound->GetMemory(), mSound->GetName());
			ASSERT(mAudioSource != 0, "Audio source seems to be still null");
			Play();
		}
	}

	cAudio::IAudioSource* source = SOURCE(mAudioSource);

	// Only continue if the sound is actually playing
	if (source != 0 && source->isPlaying())
	{
		// If the sound volume is changing, we need to adjust it inside cAudio
		if (mVolume.y != mVolume.z)
		{
			// Calculate the current fade-out factor
			float factor = (mFadeDuration > 0) ? (float)(0.001 * (time - mFadeStart)) /	mFadeDuration : 1.0f;

			if (factor < 1.0f)
			{
				// Update the volume
				mVolume.y = Interpolation::Linear(mVolume.x, mVolume.z, factor);
			}
			else
			{
				// We've reached the end of the fading process
				mVolume.y = mVolume.z;

				// If the sound was fading out, perform the target action
				if (mVolume.y < 0.0001f)
				{
					if (mAction == TargetAction::Stop)
					{
						source->stop();
						mIsPlaying = false;
						mIsPaused = false;
						mDuration = 0;
					}
					else if (mAction == TargetAction::Pause)
					{
						source->pause();
						mIsPlaying = false;
						mIsPaused = true;
					}
				}
			}
		}

		if (mIs3D)
		{
			Vector3f velocity = (mPosition - mLastPosition) * Time::GetDelta();
			cAudio::cVector3 pos (mPosition.x, mPosition.y, mPosition.z);
			cAudio::cVector3 vel (velocity.x, velocity.y, velocity.z);
			source->setPosition (pos);
			source->setVelocity(vel);
		}

		source->setVolume(mVolume.y * atten);
	}
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
		cAudio::IAudioSource* source = SOURCE(mAudioSource);
		source->seek(mDuration / 1000.0f);

		if (mIsPaused)
		{
			source->setVolume(0.0f);
			source->play();
			SetVolume(mVolume.w, 0.0f);
		}
		else if (!mIs3D)
		{
			source->setVolume(0.0f);
			source->play2d(mRepeat);
		}
		else
		{
			cAudio::cVector3 pos (mPosition.x, mPosition.y, mPosition.z);
			source->setVolume(0.0f);
			source->play3d(pos, 2.0f, mRepeat);
			source->setMinDistance(1000.0f);
			source->setMaxDistance(1000.0f);
		}
	}
	
	mIsPlaying = true;
	mIsPaused = false;
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
// Sets the 3D position of the specified sound (Should only be set once per frame)
//============================================================================================================

void SoundInstance::SetPosition (const Vector3f& position)
{
	mLastPosition = mPosition;
	mPosition = position;
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
// Sets the range of the sound x = min distance (max sound), y = max distance(no sound)
//============================================================================================================

void SoundInstance::SetRange (Vector2f& range)
{
	mRange = range;
}

//============================================================================================================
// INTERNAL: Set the volume of the sound
//============================================================================================================

void SoundInstance::_SetVolume (float volume, float calculatedVolume, float duration)
{
	mVolume.x = mVolume.y;
	mVolume.z = calculatedVolume;
	mVolume.w = volume;
	mFadeStart = Time::GetMilliseconds();
	mFadeDuration = duration;
}