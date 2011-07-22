#include "../Include/_All.h"

#include "AL/al.h"

using namespace R5;

SoundInstance::SoundInstance(Sound* sound)
	:mActiveInstancesEntry(NULL), mState(0), mVolume(1.0f), mSound(sound)
{
	alGenSources(1, &mSource);
	ASSERT(alGetError() == 0, "AL error occured");

	mSound->mRefCount++;
}

SoundInstance::~SoundInstance()
{
	mLayer->mInstances.Recycle(mInstancesEntry);

	if (mActiveInstancesEntry)
	{
		mActiveInstances->Recycle(mActiveInstancesEntry);
	}

	alDeleteSources(1, &mSource);
	ASSERT(alGetError() == 0, "AL error occured");

	mSound->mRefCount--;
}

void SoundInstance::SetVolume(float volume)
{
	mVolume = Min(volume, 1.0f);
	alSourcef(mSource, AL_GAIN, mLayer->GetVolume()*mVolume);
	ASSERT(alGetError() == 0, "AL error occured");
}

void SoundInstance::SetPosition(const Vector3f& position)
{
	alSourcefv(mSource, AL_POSITION, position);
	ASSERT(alGetError() == 0, "AL error occured");
}

void SoundInstance::SetVelocity(const Vector3f& velocity)
{
	alSourcefv(mSource, AL_VELOCITY, velocity);
	ASSERT(alGetError() == 0, "AL error occured");
}

void SoundInstance::SetSpatial(bool isSpatial)
{
	alSourcei(mSource, AL_SOURCE_RELATIVE, true);
	ASSERT(alGetError() == 0, "AL error occured");
}
