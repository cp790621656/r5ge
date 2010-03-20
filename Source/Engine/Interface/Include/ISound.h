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

	virtual R5::ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;

	// Play the specified sound at the specified position
	virtual R5::ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;

	// Set the buffer used to create instances.
	virtual void Set(const byte* buffer, uint size)=0;

	// Get the associated Audio class
	virtual const R5::IAudio* GetAudio()=0;

	virtual const String& GetName()=0;

	virtual void SetAudio (IAudio* audio)=0;
	virtual void SetName  (const String& name)=0;
};