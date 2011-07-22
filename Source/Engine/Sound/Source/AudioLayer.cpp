#include "../Include/_All.h"

using namespace R5;

AudioLayer::~AudioLayer()
{
	if (mInstances.IsValid())
	{
		LinkedList<SoundInstance*>::Entry* e;

		while ( (e = mInstances.GetFirst()) )
		{
			e->mVal->DestroySelf();
		}
	}
}

void AudioLayer::SetVolume(float volume)
{
	mVolume = Min(1.0f, volume);
	LinkedList<SoundInstance*>::Entry* instanceEntry = mInstances.GetFirst();
	if (instanceEntry)
	{
		do
		{
			volume = instanceEntry->mVal->GetVolume();
			instanceEntry->mVal->SetVolume(volume);
		}
		while ( (instanceEntry = mInstances.GetNext(instanceEntry)) );
	}
}
