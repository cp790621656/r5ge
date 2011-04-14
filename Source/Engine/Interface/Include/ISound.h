#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the Sound class
// Author: Michael Lyashenko
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

	// Get the source for the sound
	virtual void* GetSource()=0;

	// Play the sound in 2D
	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;

	// Play the specified sound in 3D
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;
};