#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class Audio;
class SoundInstance;
class StaticInstance;
class StreamedInstance;

class AudioLayer: public IAudioLayer
{
friend class Audio;
friend class SoundInstance;
friend class StaticInstance;
friend class StreamedInstance;

private:
	LinkedList<SoundInstance*> mInstances;
	float mVolume;

public:
	virtual float GetVolume() const 
		{ return mVolume; }
	virtual void SetVolume(float volume);
	
	AudioLayer():mVolume(1.0f) {}
	virtual ~AudioLayer();
};
