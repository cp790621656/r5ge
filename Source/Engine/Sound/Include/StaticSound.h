#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class SoundInstance;

class StaticSound: public Sound
{
protected:
	StaticSound() {}

protected:
	uint mBuffer;

public:
	StaticSound(const String& name, AudioData* audioData);
	virtual ~StaticSound();

	virtual SoundInstance* Instantiate();
	virtual void SetAudioData(AudioData* audioData);
};
