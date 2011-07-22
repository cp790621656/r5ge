#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class StaticInstance: public SoundInstance
{
friend class StaticSound;

protected:
	StaticInstance(){}
	StaticInstance(Sound* sound);
	virtual ~StaticInstance();

public:
	virtual bool Update()
		{ return false; }

	virtual void SetRepeat(bool repeat);
	virtual void Play();
	virtual void Pause();
	virtual void Stop();
};
