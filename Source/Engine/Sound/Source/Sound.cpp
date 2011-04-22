#include "../Include/_All.h"
#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"
#include "../Include/vorbis/vorbisfile.h"
using namespace R5;

extern AudioLibrary g_audioLibrary;

//============================================================================================================
// OpenAL reads data using callbacks. We need to specify those callbacks.
//============================================================================================================

struct MemData
{
	ConstBytePtr mBuffer;
	uint mSize;
	uint mOffset;
};

//============================================================================================================

size_t MemReadFunc (void* dataOut, size_t dataOutSize, size_t nmemb, void* dataIn)
{
	MemData* data = (MemData*)dataIn;

	uint outSize = dataOutSize * nmemb;
	uint remain = data->mSize - data->mOffset;
	if (remain < outSize) outSize = remain;

	ConstBytePtr buffer = data->mBuffer + data->mOffset;
	Memory::Extract(buffer, remain, dataOut, outSize);
	data->mOffset += outSize;
	return outSize;
}

//============================================================================================================

long MemTellFunc (void* dataIn)
{
	MemData* data = (MemData*)dataIn;
	return data->mOffset;
}

//============================================================================================================

int MemSeekFunc (void* dataIn, ogg_int64_t offset, int whence)
{
	MemData* data = (MemData*)dataIn;

	if (whence == SEEK_SET)
	{
		data->mOffset = (uint)Clamp((int)offset, 0, (int)data->mSize);
	}
	else if (whence == SEEK_END)
	{
		data->mOffset = (uint)Clamp((int)data->mSize + (int)offset, 0, (int)data->mSize);
	}
	else
	{
		data->mOffset = (uint)Clamp((int)data->mOffset + (int)offset, 0, (int)data->mSize);
	}
	return 0;
}

//============================================================================================================

int MemCloseFunc (void* dataIn)
{
	MemData* data = (MemData*)dataIn;
	data->mBuffer = 0;
	data->mOffset = 0;
	data->mSize = 0;
	return 0;
}

//============================================================================================================
// Sound Library
//============================================================================================================

Sound::~Sound()
{
	if (mBuffer)
	{
		alDeleteBuffers(1, &mBuffer);
		mBuffer = 0;
	}
}

//============================================================================================================
// Whether this is a playable sound
//============================================================================================================

bool Sound::IsValid() const
{
	if (!mBuffer && mCheckForSource)
	{
		mCheckForSource = false;
		Memory mem;

		if (mem.Load(mName))
		{
			((Sound*)this)->SetSourceData(mem.GetBuffer(), mem.GetSize());
		}
	}
	return (mBuffer != 0);
}

//============================================================================================================
// Set the source data from which the sound gets created
//============================================================================================================

bool Sound::SetSourceData (const byte* buffer, uint size)
{
	mCheckForSource = false;

	g_audioLibrary.Lock();
	{
		if (mBuffer)
		{
			alDeleteBuffers(1, &mBuffer);
			mBuffer = 0;
		}

		ov_callbacks cb;
		cb.read_func  = &MemReadFunc;
		cb.tell_func  = &MemTellFunc;
		cb.seek_func  = &MemSeekFunc;
		cb.close_func = &MemCloseFunc;

		MemData memData;
		memData.mBuffer	= buffer;
		memData.mOffset	= 0;
		memData.mSize	= size;

		OggVorbis_File ogg;
		int retVal = ov_open_callbacks(&memData, &ogg, NULL, 0, cb);
		ASSERT(retVal == 0, "ov_open_callbacks call failed");

		if (retVal)
		{
			g_audioLibrary.Unlock();
			return false;
		}

		vorbis_info* vorbisInfo;
		vorbisInfo = ov_info(&ogg, -1);

		Memory decoded;
		long bytesRead (0);
		int bitStream (0);
		char data[4096] = {0};

		for (;;)
		{
			bytesRead = ov_read(&ogg, data, sizeof(data), 0, 2, 1, &bitStream);

			if (bytesRead < 0)
			{
				ASSERT(false, "ov_read returned an error code");
				g_audioLibrary.Unlock();
				return false;
			}

			if (bytesRead == 0) break;
			decoded.Append((const void*)data, (uint)bytesRead);
		}

		ALenum format;
		format = (vorbisInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

		alGenBuffers(1, &mBuffer);
		alBufferData(mBuffer, format, decoded.GetBuffer(), decoded.GetSize(), (uint)vorbisInfo->rate);
		ov_clear(&ogg);
	}
	g_audioLibrary.Unlock();
	return true;
}

//============================================================================================================
// Play the specified in 2D
//============================================================================================================

ISoundInstance* Sound::Play (uint layer, float fadeInTime, bool repeat)
{
	return mAudio->Instantiate(this, layer, fadeInTime, repeat);
}

//============================================================================================================
// Play the specified sound at the specified position
//============================================================================================================

ISoundInstance* Sound::Play (const Vector3f& position, uint layer , float fadeInTime, bool repeat)
{
	return mAudio->Instantiate(this, position, layer, fadeInTime, repeat);
}
