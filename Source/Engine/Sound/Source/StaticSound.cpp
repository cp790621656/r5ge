#include "../Include/_All.h"

#include "AL/al.h"

using namespace R5;

StaticSound::StaticSound(const String& name, AudioData *audioData)
{
	alGenBuffers(1, &mBuffer);
	ASSERT(alGetError() == 0, "AL error occured");

	mName = name;
	SetAudioData(audioData);
	delete audioData;
}

StaticSound::~StaticSound()
{
	alDeleteBuffers(1, &mBuffer);
	ASSERT(alGetError() == 0, "Coudn't delete sound");
}

SoundInstance* StaticSound::Instantiate()
{
	StaticInstance* inst = new StaticInstance(this);

	alSourcei(inst->mSource, AL_BUFFER, mBuffer);
	ASSERT(alGetError() == 0, "AL error occured");
	
	return inst;
}

void StaticSound::SetAudioData(AudioData* audioData)
{
	Memory dataOut;
	audioData->SetChunkSize(0);
	audioData->Decode(dataOut);

	ALenum format = (audioData->GetChannels() == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;	

	alBufferData(mBuffer, format, dataOut.GetBuffer(), dataOut.GetSize(), audioData->GetRate());
	ASSERT(alGetError() == 0, "AL error occured");
}
