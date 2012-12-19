#include "../Include/_All.h"

using namespace R5;

//============================================================================================================
// Ogg/Vorbis callbacks for decoding a stream in memory
//============================================================================================================

typedef struct _Data
{
	uint		streamPointer;
	Memory*		streamData;
} _Data;


size_t MemRead(void* outBuffer, size_t size, size_t nmemb, void* dataSource)
{
	_Data* data = ((_Data*)dataSource);

	uint dataSize = Min((uint)(size * nmemb), data->streamData->GetSize() - data->streamPointer);

	if (dataSize)
	{
		void* inBuffer = (void*)(data->streamData->GetBuffer() + data->streamPointer);
		data->streamPointer += dataSize;

		memcpy(outBuffer, inBuffer, dataSize);
	}

	return dataSize;
}

int MemSeek(void* dataSource, ogg_int64_t offset, int whence)
{
	_Data* data = ((_Data*)dataSource);

	if (whence == SEEK_SET)
	{
		data->streamPointer = Min((uint)offset, data->streamData->GetSize());
	}
	else if (whence == SEEK_END)
	{
		data->streamPointer = data->streamData->GetSize() - Min((uint)offset, data->streamData->GetSize());
	}
	else
	{
		data->streamPointer = Min((uint)(offset + data->streamPointer), data->streamData->GetSize());
	}

	return 0;
}

int MemClose(void *dataSource)
{
	delete (_Data*)dataSource;
	return 0;
}

long int MemTell(void* dataSource)
{
	return ((_Data*)dataSource)->streamPointer;
}

//============================================================================================================

AudioData::AudioData()
	: mEOF(false), mChunkSize(0), mOggFile(NULL)
{
}

AudioData::~AudioData()
{
	if (mOggFile != NULL)
	{
		ov_clear(mOggFile);
		delete mOggFile;
	}
}

//============================================================================================================
// Get the length of the audio stream in seconds
//============================================================================================================

long AudioData::GetLength() const
{
	return (long)ov_pcm_total(mOggFile, -1) << 1;
}

//============================================================================================================
// Load the file
//============================================================================================================

bool AudioData::Load(const String& name)
{
	if (mData.Load(name))
	{
		ov_callbacks cb = { MemRead, MemSeek, MemClose, MemTell };

		_Data* data = new _Data;
		data->streamPointer = 0;
		data->streamData = &mData;

		mOggFile = new OggVorbis_File;
		int retVal = ov_open_callbacks((void*)data, mOggFile, NULL, 0, cb);
		ASSERT(retVal == 0, "ov_open_callbacks failed");

		vorbis_info* vi = ov_info(mOggFile, -1);
		mChannels = vi->channels;
		mRate = vi->rate;

		return true;
	}
	return false;
}

//============================================================================================================
// Decodes the stream. If mChunkSize was set to a value other than 0, then a maximum of mChunkSize of bytes
// are decoded and placed in the buffer, otherwise everything from the current position to the end is decoded
//============================================================================================================

void AudioData::Decode(Memory& outBuffer)
{
	outBuffer.Clear();

	if (!mEOF)
	{
		if (mChunkSize) 
			outBuffer.Reserve(mChunkSize);
		else 
			outBuffer.Reserve(GetLength());

		int bytesRead = 0;
		uint size = 0;
		do
		{
			int stream;
			size = outBuffer.GetAllocated() - outBuffer.GetSize();
			if (size)
			{
				bytesRead = ov_read(mOggFile, (char*)outBuffer.GetBuffer() + outBuffer.GetSize(), size, 0, 2, 1, &stream);
				outBuffer.Resize(outBuffer.GetSize() + bytesRead);
			}
			ASSERT(bytesRead >= 0, "ov_read returned an error code");
		}
		while (bytesRead > 0 && size > 0);

		if (ov_pcm_tell(mOggFile) == ov_pcm_total(mOggFile, -1)) 
			mEOF = true;
	}
}

//============================================================================================================
// Resets the data stream, so that decoding starts from the beginning of the stream.
//============================================================================================================

void AudioData::Reset()
{
	mEOF = false;
	ov_raw_seek(mOggFile, 0);
}
