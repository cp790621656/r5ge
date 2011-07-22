#include "../Include/_All.h"

#include <AL/al.h>

using namespace R5;

StaticInstance::StaticInstance(Sound* sound): 
	SoundInstance(sound)
{
}

StaticInstance::~StaticInstance()
{
	alSourceStop(mSource);
	alSourcei(mSource, AL_BUFFER, 0);
	ASSERT(alGetError() == 0, "AL error occured");
}

void StaticInstance::SetRepeat(bool repeat)
{
	alSourcei(mSource, AL_LOOPING, repeat);
	ASSERT(alGetError() == 0, "AL error occured");
}

void StaticInstance::Play()
{
	mState = State::Playing;

	alSourcePlay(mSource);
	ASSERT(alGetError() == 0, "AL error occured");
}

void StaticInstance::Pause()
{
	mState = State::Paused;

	alSourcePause(mSource);
	ASSERT(alGetError() == 0, "AL error occured");
}

void StaticInstance::Stop()
{
	mState = State::Stopped;

	ASSERT(alGetError() == 0, "AL error occured");
	alSourceStop(mSource);
}
