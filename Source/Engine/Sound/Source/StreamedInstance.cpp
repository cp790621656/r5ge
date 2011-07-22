#include "../Include/_All.h"

#include "AL/al.h"

using namespace R5;

StreamedInstance::StreamedInstance(Sound* sound): 
	mBuffersProcessed(0), mChunk(0), SoundInstance(sound)
{
	alGenBuffers(2, mBuffers);
	ASSERT(alGetError() == 0, "AL error occured");
}

StreamedInstance::~StreamedInstance()
{
	alSourceStop(mSource);

	alSourcei(mSource, AL_BUFFER, 0);
	ASSERT(alGetError() == 0, "AL error occured");

	alDeleteBuffers(2, mBuffers);
	ASSERT(alGetError() == 0, "AL error occured");
}

//============================================================================================================
// Initializes the sound, preloading the two buffers with the first two chunks of the stream
//============================================================================================================

void StreamedInstance::Init()
{
	Memory dataOut;
	ALenum format = (((StreamedSound*)mSound)->GetAudioData()->GetChannels() == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;	

	alSourcei(mSource, AL_BUFFER, 0);
	ASSERT(alGetError() == 0, "AL error occured");

	((StreamedSound*)mSound)->GetAudioData()->Reset();
	((StreamedSound*)mSound)->GetAudioData()->Decode(dataOut);

	alBufferData(mBuffers[0], format, dataOut.GetBuffer(), dataOut.GetSize(), ((StreamedSound*)mSound)->GetAudioData()->GetRate());
	ASSERT(alGetError() == 0, "AL error occured");

	((StreamedSound*)mSound)->GetAudioData()->Decode(dataOut);

	alBufferData(mBuffers[1], format, dataOut.GetBuffer(), dataOut.GetSize(), ((StreamedSound*)mSound)->GetAudioData()->GetRate());
	ASSERT(alGetError() == 0, "AL error occured");

	alSourceQueueBuffers(mSource, 2, mBuffers);
	ASSERT(alGetError() == 0, "AL error occured");
}

//============================================================================================================
// Updates the sound, loading new portions as it's being played. 
// Returns false when the sound has finished playing
//============================================================================================================

bool StreamedInstance::Update()
{
	if (mState != State::Playing) return false;

	int queued;
	alGetSourcei(mSource, AL_BUFFERS_QUEUED, &queued);
	ASSERT(alGetError() == 0, "AL error occured");

	if (queued)
	{
		int processed;
		alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
		ASSERT(alGetError() == 0, "AL error occured");

		while (processed--)
		{
			uint processedBuffer;
			alSourceUnqueueBuffers(mSource, 1, &processedBuffer);
			ASSERT(alGetError() == 0, "AL error occured");

			if (((StreamedSound*)mSound)->GetAudioData()->IsEOF() && mRepeat)
			{
				((StreamedSound*)mSound)->GetAudioData()->Reset();
			}

			if (!((StreamedSound*)mSound)->GetAudioData()->IsEOF())
			{
				Memory dataOut;
				ALenum format = (((StreamedSound*)mSound)->GetAudioData()->GetChannels() == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;	

				((StreamedSound*)mSound)->GetAudioData()->Decode(dataOut);

				alBufferData(processedBuffer, format, dataOut.GetBuffer(), dataOut.GetSize(), ((StreamedSound*)mSound)->GetAudioData()->GetRate());
				ASSERT(alGetError() == 0, "AL error occured");

				alSourceQueueBuffers(mSource, 1, &processedBuffer);
				ASSERT(alGetError() == 0, "AL error occured");
			}
		}

		int streamState;
		alGetSourcei(mSource, AL_SOURCE_STATE, &streamState);

		if (streamState == AL_STOPPED)
		{
			alSourcePlay(mSource);
		}

		return true;
	}
	else
	{
		// Load the buffers with initial data in case the sound is to be played again.
		Init();
		mState = State::Stopped;
		mActiveInstancesEntry = NULL;
		return false;
	}
}

void StreamedInstance::SetRepeat(bool repeat)
{
	mRepeat = repeat;
}

void StreamedInstance::Play() 
{
	if (mState != State::Playing)
	{
		mState = State::Playing;

		alSourcePlay(mSource);
		ASSERT(alGetError() == 0, "AL error occured");

		if (!mActiveInstancesEntry)
		{
			mActiveInstancesEntry = mActiveInstances->GetUnused();
			mActiveInstances->Expand() = this;	
		}
	}
}

void StreamedInstance::Pause()
{
	if (mState == State::Playing)
	{
		mState = State::Paused;

		alSourcePause(mSource);
		ASSERT(alGetError() == 0, "AL error occured");
	}
}

void StreamedInstance::Stop() 
{
	if (mState == State::Playing)
	{
		mState = State::Stopped;

		alSourceStop(mSource);
		ASSERT(alGetError() == 0, "AL error occured");

		Init();
	}
}
