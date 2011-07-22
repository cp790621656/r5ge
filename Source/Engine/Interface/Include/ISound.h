#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the Sound class
// Author: Michael Lyashenko
//============================================================================================================

struct ISound
{
	virtual ~ISound() {}

	// Get the name of the sound
	virtual const String& GetName() const=0;
};
