#include "../Include/_All.h"
#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"
using namespace R5;

//============================================================================================================
// Sound Instance library
//============================================================================================================

using namespace R5;

//============================================================================================================

SoundInstance::~SoundInstance()
{
	alDeleteSources(1, &mSource);
}

//============================================================================================================
// Update the sound
//============================================================================================================

void SoundInstance::Update(ulong time)
{
	if (mIsPlaying)
	{
		mDuration += Time::GetDeltaMS();
	}

	float atten = 1.0f;
		
	if (mIs3D)
	{
		atten = 1.0f - Min((mPosition - mSound->GetAudio()->GetListener()).Magnitude() / 
			(mRange.y - mRange.x), 1.0f);
		atten *= atten;
	}

	// Only continue if the sound is actually playing
	if (mSource != 0 && mIsPlaying)
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
						//source->setIsPaused(true);
						alSourceStop(mSource);
						mIsPlaying = false;
						mIsPaused = false;
						mDuration = 0;
					}
					else if (mAction == TargetAction::Pause)
					{
						//source->setIsPaused(true);
						alSourcePause(mSource);
						mIsPlaying = false;
						mIsPaused = true;
					}
				}
			}
		}

		if (mIs3D)
		{
			alSourcefv(mSource, AL_POSITION, mPosition);
			alSourcefv(mSource, AL_VELOCITY, (mPosition - mLastPosition) * Time::GetDelta());
		}
		alSourcef(mSource, AL_GAIN, mVolume.y * atten);
	}
}

//============================================================================================================
// Destoryes the sound instance releasing the memory
//============================================================================================================

void SoundInstance::DestroySelf()
{
	IAudio* aud = mSound->GetAudio();
	((Audio*)aud)->ReleaseInstance(this);
}

//============================================================================================================
// Play the sound
//============================================================================================================

void SoundInstance::Play()
{
	if (mSource !=0)
	{
		alSourcef(mSource, AL_GAIN, 0.0f);

		if (!mIsPlaying)
		{
			alSourcePlay(mSource);
			SetVolume(mVolume.w, 0.0f);
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

void SoundInstance::SetVolume (float volume, float fadeDuration)
{
	float layerVolume = mSound->GetAudio()->GetLayerVolume(mLayer);
	
	_SetVolume (volume, layerVolume * volume, fadeDuration);
}

//============================================================================================================
// Sets whether the sound will repeat after it ends
//============================================================================================================

void SoundInstance::SetRepeat (bool repeat)
{
	mRepeat = repeat;

	if (mSource != 0)
	{
		alSourcei(mSource, AL_LOOPING, repeat);
	}
}

//============================================================================================================
// Sets the range of the sound x = min distance (max sound), y = max distance(no sound)
//============================================================================================================

void SoundInstance::SetRange (const Vector2f& range)
{
	mRange = range;
}

//============================================================================================================
// The effect that is going to be played on this sound. Null will disable the effect
//============================================================================================================

void SoundInstance::SetEffect (byte effect)
{
// TODO: Implement this
//	if (mEffect != effect)
//	{
//		mEffect = effect;
//
//		if (effect != 0)
//		{
//			if (effect == Effect::Auditorium)
//			{
//				return;
//			}
//		}
//	}
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
