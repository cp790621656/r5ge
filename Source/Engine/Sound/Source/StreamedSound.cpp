#include "../Include/_All.h"

using namespace R5;

//============================================================================================================
// Creates a new instance of streamed sound object
//============================================================================================================

SoundInstance* StreamedSound::Instantiate()
{
	StreamedInstance* inst = new StreamedInstance(this);
	inst->Init();
	
	return inst;
}

StreamedSound::StreamedSound(const String& name, AudioData *audioData)
{
	mName = name;
	mAudioData = audioData;
	mAudioData->SetChunkSize(4096);
}

StreamedSound::~StreamedSound()
{
	delete mAudioData;
}

void StreamedSound::SetAudioData(AudioData* audioData)
{
	mAudioData = audioData;
}

AudioData* StreamedSound::GetAudioData() 
{
	return mAudioData;
}
