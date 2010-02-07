#include "../Include/_All.h"

#ifdef _WINDOWS
	#include <windows.h>
	#pragma comment(lib, "winmm.lib")
#else
	#include <sys/time.h>
  
	inline ulong timeGetTime()
	{
		timeval t;
		gettimeofday(&t, 0);
		return (ulong)(t.tv_sec * 1000 + t.tv_usec / 1000);
	}
#endif

using namespace R5;

//======================================================================================================

float	g_time			= 0.0f;
float	g_delta			= 0.0f;
ulong	g_currentTime	= 0;
ulong	g_deltaMS		= 0;
uint	g_fps			= 0;

float	Time::GetTime()			{ return g_time; }
float	Time::GetDelta()		{ return g_delta; }
ulong	Time::GetMilliseconds()	{ return g_currentTime; }
ulong	Time::GetDeltaMS()		{ return g_deltaMS; }
uint	Time::GetFPS()			{ return g_fps; }

//======================================================================================================
// Updates the time in milliseconds since the application was started, making Time::GetMilliseconds()
// function as quick as possible. Time::Update() should be called at the beginning of the message
// processing loop, or some other frequently visited location.
//======================================================================================================

void Time::Update()
{
	static ulong startTime = ::timeGetTime();
	g_currentTime = ::timeGetTime() - startTime;
	
	static ulong lastTime = g_currentTime;
	g_deltaMS = g_currentTime - lastTime;
	lastTime = g_currentTime;

	g_time  = 0.001f * g_currentTime;
	g_delta = 0.001f * g_deltaMS;
}

//======================================================================================================
// Increments the frame count -- should be called at the end of every drawn frame
//======================================================================================================

void Time::IncrementFPS()
{
	static uint count = 0;
	static ulong nextFPSUpdate = 250;

	++count;

	if ( nextFPSUpdate < g_currentTime )
	{
		nextFPSUpdate = g_currentTime + 250;
		g_fps = count * 4;
		count = 0;
	}
}