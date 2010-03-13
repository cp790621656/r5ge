#pragma once

//============================================================================================================
//         R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// cAudio IAudioManager class wrapper
//============================================================================================================

class Audio : public IAudio
{
protected:
	typedef Array<Sound*> PlayingSounds;

	struct AudioLayer
	{
		uint			mLayer;
		float			mVolume;
		PlayingSounds	mPlayingSound;

		AudioLayer(uint layer, float volume) : mLayer(layer), mVolume(volume) {} 
	};

	PointerHash<AudioLayer>	mAudioLayers;
	PointerHash<Sound>		mAudioFiles;
	bool					mSerializable;

public:

	Audio();
	~Audio();

	virtual void Update();

	virtual void Play (
		String& fileName, 
		uint layer = 0, 
		bool loop = false, 
		float volume = 1.0f,
		float duration = 0.0f);

	virtual void Play (
		String& fileName,
		Vector3f position,
		uint layer = 0, 
		bool loop = false, 
		float volume = 1.0f,
		float duration = 0.0f);

	virtual void	Stop		(String& fileName, float duration = 0.25f);
	virtual void	Pause		(String& fileName, float duration = 0.25f);
	virtual void	Release		(String& fileName){}
	
	virtual void	SetSoundVolume		(String& fileName, float volume, float duration = 0.25f);
	virtual void	SetLayerVolume		(uint layer, float volume, float duration = 0.25f);
	virtual void	SetListenerPosition (const Vector3f position);
	virtual void	SetSoundLooping		(String& fileName, bool loop);
	virtual void	SetSoundPosition	(String& fileName, Vector3f position);
	
	virtual const float	GetLayerVolume	(uint layer) const;
	virtual const float	GetSoundVolume	(String& fileName) const;

private:
	void	Stop		(Sound* sound, float duration = 0.25f);
	void	Pause		(Sound* sound, float duration = 0.25f);
	void	SetVolume	(Sound* sound, float volume, float duration = 0.25f);
	void	SetPosition	(Sound* sound, Vector3f position);
	void	SetLooping	(Sound* sound, bool loop);
	void	PlaySound	(String& fileName, uint layer, bool loop, float volume, float duration);
	
	Sound* CreateAudioSource (String& fileName);
};