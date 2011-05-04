#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the Sound class
// Author: Michael Lyashenko
//============================================================================================================

struct IAudio;
struct ISound
{
	virtual ~ISound() {}

	// Get the name of the sound
	virtual const String& GetName() const=0;

	// Get the associated Audio class
	virtual IAudio* GetAudio()=0;

	// Whether this is a playable sound
	virtual bool IsValid() const=0;

	// Set the source data from which the sound gets created
	virtual bool SetSourceData (const byte* buffer, uint size)=0;

	// Play the sound in 2D
	virtual ISoundInstance* Play (uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;

	// Play the specified sound in 3D
	virtual ISoundInstance* Play (const Vector3f& position, uint layer = 0, float fadeInTime = 0.0f, bool repeat = false)=0;
};
