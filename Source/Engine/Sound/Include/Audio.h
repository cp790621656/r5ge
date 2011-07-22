#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class SoundInstance;
class Sound;

class Audio: public IAudio
{
private:
	typedef AudioLayer* AudioLayerPtr;

protected:
	LinkedList<SoundInstance*> mActiveInstances;

	Hash<AudioLayer>		mLayers;
	LinkedList<Sound*>		mSounds;

public:
	Audio();

	virtual AudioLayer* GetLayer(uint layer) 
				{ return &mLayers[layer]; }

	virtual void SetListener(const Vector3f& position, const Vector3f& direction, const Vector3f& up, const Vector3f& velocity);

	virtual ISound* GetSound(const String& name, bool createIfMissing = true);
	
	virtual ISoundInstance* Instantiate(ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat = false);
	virtual ISoundInstance* Instantiate(ISound* sound, uint layer, float fadeInTime, bool repeat = false);
	
	virtual void Release();
	virtual bool Release(ISound* sound);

	virtual void Update();
	
	virtual ~Audio();
	

// INTERNAL
private:
	SoundInstance* _Instantiate(Sound* sound, uint layer, bool repeat);

};
