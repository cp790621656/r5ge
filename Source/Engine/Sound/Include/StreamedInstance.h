#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class StreamedInstance: public SoundInstance
{
friend class StreamedSound;

protected:
	uint			mBuffers[2];
	int				mBuffersProcessed;
	bool			mRepeat;
	uint			mChunk;
	uint			mNextChunk;

protected:
	StreamedInstance() {}
	StreamedInstance(Sound* sound);
	virtual ~StreamedInstance();

public:
	virtual void SetRepeat(bool repeat);
	virtual void Init();
	virtual bool Update();

	virtual void Play();
	virtual void Pause();
	virtual void Stop();
};
