#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Assert macro -- will be automatically omitted in non-debug versions
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _DEBUG

#ifdef _WINDOWS
#define INT3 __asm { int 3 }
#else
#define INT3 asm ( "int $3\t\n" );
#endif

	#define ASSERT( exp, description ) \
	{ \
		static bool keepChecking = true; \
		if ( (((bool)(exp)) == false) && keepChecking ) \
		{ \
			System::Log("[ASSERT]  %s", description); \
			System::Log("          - Function: %s", __FUNCTION__); \
			System::Log("          - Filename: %s (Line: %u)", __FILE__, __LINE__); \
			System::FlushLog();	\
			if ( R5::Thread::AssertWindow(description, __LINE__, __FILE__, keepChecking) ) { INT3 }	\
		} \
	}
	#define WARNING(text)	ASSERT(false, text)
	#define DEBUG(text)		R5::Thread::MessageWindow(text);

#else

	#define ASSERT( exp, description )
	#define WARNING(text)	R5::System::Log("[WARNING] %s", text);
	#define DEBUG(text)

#endif
