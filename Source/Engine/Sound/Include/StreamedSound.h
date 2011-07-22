#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class StreamedSound: public Sound
{
protected:
	StreamedSound() {}

protected:
	AudioData *mAudioData;

public:
	StreamedSound(const String& name, AudioData* audioData);
	virtual ~StreamedSound();

	AudioData* GetAudioData();

	virtual SoundInstance* Instantiate();
	virtual void SetAudioData(AudioData* audioData);
};
