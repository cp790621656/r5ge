#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

namespace Time
{
	void	Update();			// Sets the saved time using timeGetTime() or GetTickCount()
	void	IncrementFPS();		// Increments the framerate by 1 (should be called once per frame)
	float	GetTime();			// Timestamp in seconds
	float	GetDelta();			// Seconds since last update
	ulong	GetMilliseconds();	// Timestamp in milliseconds
	ulong	GetDeltaMS();		// Milliseconds since the last update
	uint	GetFPS();			// Current framerate
};