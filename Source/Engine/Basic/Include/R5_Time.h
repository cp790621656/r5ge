#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================

namespace Time
{
	void	Update();			// Sets the saved time using timeGetTime() or GetTickCount()
	void	IncrementFPS();		// Increments the framerate by 1 (should be called once per frame)
	float	GetTime();			// Timestamp in seconds
	float	GetDelta();			// Seconds since last update
	double	GetSeconds();		// Precise timestamp in seconds
	ulong	GetMilliseconds();	// Timestamp in milliseconds
	ulong	GetDeltaMS();		// Milliseconds since the last update
	uint	GetFPS();			// Current framerate
};