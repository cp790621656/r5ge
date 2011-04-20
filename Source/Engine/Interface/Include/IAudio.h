#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the Audio controller class
// Author: Michael Lyashenko
//============================================================================================================

struct IAudio
{
	R5_DECLARE_INTERFACE_CLASS("Audio");

	virtual ~IAudio() {}

	// Release all audio resources
	virtual void Release()=0;

	// Update notification
	virtual void Update()=0;
	
	// Release all resources associated with the specified sound
	virtual void Release (ISound* sound)=0;

	// Retrieves the audio listener position
	virtual const Vector3f& GetListener() const=0;

	// Sets the sound listener pos/dir/up (usually should be the camera)
	virtual void SetListener (const Vector3f& position, const Vector3f& dir, const Vector3f& up)=0;

	// Changes the volume of an entire sound layer (music layer, for example)
	virtual void SetLayerVolume (uint layer, float volume, float duration = 0.25f)=0;

	// Gets the volume of the specified layer
	virtual const float	GetLayerVolume (uint layer) const=0;

	// Get the sound by name
	virtual ISound* GetSound (const String& name, bool createIfMissing = true)=0;

	// Release the soundinstance instance
	virtual void ReleaseInstance (ISoundInstance* sound)=0;

	// Create a 2D sound instance
	virtual ISoundInstance* Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat)=0;

	// Create a 3D sound instance
	virtual ISoundInstance* Instantiate (ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat)=0;
};
