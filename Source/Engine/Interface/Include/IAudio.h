#pragma once

//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic interface for the Audio controller class
//============================================================================================================

struct IAudio
{
	R5_DECLARE_INTERFACE_CLASS("Audio");

	virtual ~IAudio() {}

	// Release all audio resources
	virtual void Release()=0;

	// Update notification
	virtual void Update()=0;

	// Adds a new sound to the library
	virtual void Add (const String& name, const byte* buffer, uint size)=0;

	// Play the specified sound in 2D
	virtual void Play (const String& name,
		uint	layer		= 0, 
		bool	loop		= false, 
		float	volume		= 1.0f,
		float	fadeInTime	= 0.0f)=0;

	// Play the specified sound at the specified position
	virtual void Play (const String& name, const Vector3f& position,
		uint	layer		= 0, 
		bool	loop		= false, 
		float	volume		= 1.0f,
		float	fadeInTime	= 0.0f)=0;

	// Pause the playback of the specified sound temporarily, fading it out over the specified duration
	virtual void Pause (const String& name, float duration = 0.25f)=0;

	// Stop playing the specified sound, fading it out over the specified duration
	virtual void Stop (const String& name, float duration = 0.25f)=0;
	
	// Release all resources associated with the specified sound
	virtual void Release (const String& name)=0;

	// Sets the sound listener position (usually should be the camera's position)
	virtual void SetListenerPosition(const Vector3f& position)=0;

	// Changes the volume of an entire sound layer (music layer, for example)
	virtual void SetLayerVolume (uint layer, float volume, float duration = 0.25f)=0;

	// Changes the volume of the specified sound
	virtual void SetSoundVolume	(const String& name, float volume, float duration = 0.25f)=0;

	// Sets whether the sound will repeat after it ends
	virtual void SetSoundRepeat	(const String& name, bool repeat)=0;

	// Sets the 3D position of the specified sound
	virtual void SetSoundPosition (const String& name, const Vector3f& position)=0;

	// Gets the volume of the specified layer
	virtual const float	GetLayerVolume	(uint layer) const=0;

	// Gets the volume of the specified sound
	virtual const float	GetSoundVolume	(const String& name) const=0;
};