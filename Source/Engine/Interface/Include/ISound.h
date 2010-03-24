#pragma once

//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic interface for the Sound class
//============================================================================================================

struct IAudio;

struct ISound
{
	R5_DECLARE_INTERFACE_CLASS("ISound");

	virtual ~ISound() {}

	// Get the name of the sound
	virtual const String& GetName() const=0;

	// Get the associated Audio class
	virtual IAudio* GetAudio()=0;
	
	// Get the memory buffer for the sound
	virtual Memory& GetMemory()=0;

	// Play the sound in 2D
	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;

	// Play the specified sound in 3D
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;

	// Set the buffer used to create instances.
	virtual void Set(const byte* buffer, uint size)=0;
};