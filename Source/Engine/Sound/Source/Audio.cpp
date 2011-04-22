#include "../Include/_All.h"
#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"
#include "../Include/vorbis/vorbisfile.h"
using namespace R5;

//============================================================================================================

#ifdef _WINDOWS
  #pragma comment(lib, "ogg.lib")
  #pragma comment(lib, "vorbis.lib")
  #pragma comment(lib, "OpenAL32.lib")
#endif

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
// Initialize the Audio library
//============================================================================================================

uint g_refCount = 0;

Audio::Audio() : mAudioLib(0)
{
	if (++g_refCount == 1) alcInit();

	ALCdevice* device = alcOpenDevice(NULL);
	ASSERT(device != 0, "Couldn't open the default OpenAL device");
	
	ALCcontext* context = alcCreateContext(device, NULL);
	ASSERT(context != 0, "Couldn't create the default context");

	ALCboolean retval = alcMakeContextCurrent(context);
	ASSERT(retval != 0, "Couldn't make the context current");
	
	alDistanceModel(AL_NONE);
	ASSERT(alGetError() == AL_NO_ERROR, "error");
}

//============================================================================================================

Audio::~Audio()
{
	Release();
	ALCcontext* context = alcGetCurrentContext();
	ALCdevice* device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);

	if (--g_refCount == 0) alcRelease();
}

//============================================================================================================
// Release all audio resources
//============================================================================================================

void Audio::Release()
{
	Lock();
	{
		mLayers.Release();
		mLibrary.Release();
	}
	Unlock();
}

//============================================================================================================
// Update the different sounds every frame.
//============================================================================================================

void Audio::Update()
{
	Lock();
	{
		ulong currentTime = Time::GetMilliseconds();
		PointerArray<AudioLayer>& values = mLayers.GetAllValues();

		FOREACH(b, values)
		{
			AudioLayer* audioLayer = values[b];

			FOREACH(j, audioLayer->mSounds)
			{
				SoundInstance* sound = (SoundInstance*)audioLayer->mSounds[j];
				sound->Update(currentTime);
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Release all resources associated with the specified sound
//============================================================================================================

void Audio::Release (ISound* sound)
{
	Lock();
	{
		if (sound != 0) _Release(sound);
	}
	Unlock();
}

//============================================================================================================
// Sets the sound listener pos/dir/up (usually should be the camera)
//============================================================================================================

void Audio::SetListener (const Vector3f& position, const Vector3f& dir, const Vector3f& up)
{
	mPos = position;
	alListenerfv(AL_POSITION, position);

	ALfloat orient[] = {dir.x, dir.y, dir.z, up.x, up.y, up.z};
	alListenerfv(AL_ORIENTATION, orient);
}

//============================================================================================================
// Set the layers volume
//============================================================================================================

void Audio::SetLayerVolume (uint layer, float volume, float duration)
{
	Lock();
	{
		AudioLayer* audioLayer = mLayers.GetIfExists(layer);
		
		// If the audio layer isn't found create one.
		if (audioLayer == 0)
		{
			AudioLayer* al = new AudioLayer(layer,volume);
			mLayers[layer] = al;
			audioLayer = mLayers.GetIfExists(layer);
		}

		audioLayer->mVolume = volume;

		PlayingSounds& sounds = audioLayer->mSounds;
		
		// For all the playing sounds update there volume level
		if (sounds.IsValid())
		{
			for (uint b = sounds.GetSize(); b > 0; )
			{
				ISoundInstance* sound = sounds[--b];
				sound->SetVolume(sound->GetVolume(), duration);
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Get the volume of a layer if the layer exists
//============================================================================================================

const float Audio::GetLayerVolume (uint layer) const
{
	float volume (1.0f);
	AudioLayer* audioLayer = mLayers.GetIfExists(layer);
	if (audioLayer != 0) volume = audioLayer->mVolume;
	return volume;
}

//============================================================================================================
// Creates a AudioSource
//============================================================================================================

R5::ISound* Audio::GetSound (const String& name, bool createIfMissing)
{
	SoundPtr sound = mLibrary.Find(name);

	if (sound == 0 && createIfMissing)
	{
		Memory mem;

		if (mem.Load(name))
		{
			Lock();
			{
				ov_callbacks cb;
				cb.read_func  = &MemReadFunc;
				cb.tell_func  = &MemTellFunc;
				cb.seek_func  = &MemSeekFunc;
				cb.close_func = &MemCloseFunc;

				MemData memData;
				memData.mBuffer		= mem.GetBuffer();
				memData.mOffset		= 0;
				memData.mSize		= mem.GetSize();

				OggVorbis_File ogg;
				int retVal = ov_open_callbacks(&memData, &ogg, NULL, 0, cb);
				ASSERT(retVal == 0, "ov_open_callbacks call failed");

				if (!retVal)
				{
					uint buffer;
					alGenBuffers(1, &buffer);

					vorbis_info* vorbisInfo;
					vorbisInfo = ov_info(&ogg, -1);

					Memory decoded;
					long bytesRead (0);
					int bitStream (0);
					char data[4096] = {0};

					do
					{
						bytesRead = ov_read(&ogg, data, sizeof(data), 0, 2, 1, &bitStream);
						ASSERT(bytesRead >= 0, "ov_read returned an error code");
						decoded.Append((const void*)data, (uint)bytesRead);
					}
					while (bytesRead > 0);

					ALenum format;
					format = (vorbisInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

					alBufferData(buffer, format, decoded.GetBuffer(), decoded.GetSize(), (uint)vorbisInfo->rate);

					ov_clear(&ogg);

					sound = mLibrary.AddUnique(name);
					sound->SetAudio(this);
					sound->SetSource(buffer);
				}
			}
			Unlock();
		}
	}
	return sound;
}

//============================================================================================================
// Releases the cAudio Sound source.
//============================================================================================================

void Audio::ReleaseInstance(ISoundInstance* sound)
{
	Lock();
	{
		SoundInstance* inst = (SoundInstance*)sound;
		AudioLayer* audioLayer = _GetAudioLayer(inst->mLayer, 1.0f);
		audioLayer->mSounds.Remove(sound);
		delete sound;
	}
	Unlock();
}

//============================================================================================================
// Returns a newly created 2D sound source. Needs to return void* as irrklang is not defined outside this
// class and the SoundInstance class
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat)
{
	SoundInstance* soundInst = _Instantiate((Sound*)sound, layer, fadeInTime, repeat);

	uint source;
	alGenSources(1, &source);
	alSourcei( source, AL_BUFFER, ((Sound*)sound)->mBuffer );
	alSourcei( source, AL_LOOPING, (uint)repeat);
	alSourcef( source, AL_GAIN, 0.0f );
	alSourcei( source, AL_SOURCE_RELATIVE, 1 );
	alSourcePlay(source);
	soundInst->mSource = source;
	return soundInst;
}

//============================================================================================================
// Returns a newly created 3D sound source.
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat)
{
	SoundInstance* soundInst = _Instantiate((Sound*)sound, layer, fadeInTime, repeat);

	uint source;
	alGenSources(1, &source);
	alSourcefv( source, AL_POSITION, position );
	alSourcei( source, AL_BUFFER, ((Sound*)sound)->mBuffer );
	alSourcei( source, AL_LOOPING, (uint)repeat);
	alSourcef( source, AL_GAIN,	0.0f );
	alSourcePlay(source);
	soundInst->mSource = source;
	soundInst->mIs3D = true;
	return soundInst;
}

//============================================================================================================
// INTERNAL: Release the specified sound
//============================================================================================================

void Audio::_Release (ISound* sound)
{
	if (sound != 0)
	{
		mLibrary.Delete(R5_CAST(Sound, sound));
	}
}

//============================================================================================================
// INTERNAL: Retrieves the specified audio layer
//============================================================================================================

Audio::AudioLayer* Audio::_GetAudioLayer (uint layer, float volume)
{
	AudioLayerPtr& ptr = mLayers[layer];
	if (ptr == 0) ptr = new AudioLayer(layer, volume);
	return ptr;
}

//============================================================================================================
// INTERNAL: Set common properties for a sound
//============================================================================================================

SoundInstance* Audio::_Instantiate (Sound* sound, uint layer, float fadeInTime, bool repeat)
{
	String name = sound->GetName();
	SoundInstance* soundInst = new SoundInstance();
	
	soundInst->mSound = sound;
	soundInst->mLayer = layer;
	soundInst->mIsPlaying = true;
	soundInst->mIsPaused = false;

	Lock();
	{
		AudioLayer* audioLayer = _GetAudioLayer(layer, 1.0f);
		PlayingSounds& playing = audioLayer->mSounds;
		soundInst->SetVolume(1.0f, fadeInTime);
		soundInst->SetRepeat(repeat);
		playing.Expand() = soundInst;
	}
	Unlock();
	return soundInst;
}
