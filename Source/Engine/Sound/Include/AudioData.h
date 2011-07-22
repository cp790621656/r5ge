#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

#define OV_EXCLUDE_STATIC_CALLBACKS

#include <vorbis/vorbisfile.h>

#pragma once

class AudioData
{
private:
	Memory	mData;

	uint	mChannels;
	uint	mRate;

	uint	mChunkSize;
	bool	mEOF;

	OggVorbis_File*	mOggFile;

public:
	AudioData();
	~AudioData();

	uint GetChannels()
		{ return mChannels; }
	uint GetRate()
		{ return mRate; }
	void SetChunkSize(uint size)
		{ mChunkSize = size; }
	bool IsEOF()
		{ return mEOF; }

	long GetLength();
	bool Load(const String& name);
	void Decode(Memory& dataOut);
	void Reset();
};

