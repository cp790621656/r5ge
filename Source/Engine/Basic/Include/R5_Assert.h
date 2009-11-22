#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Assert macro -- will be automatically omitted in non-debug versions
//============================================================================================================

#ifdef _DEBUG

	#define ASSERT( exp, description )											\
	{																			\
		static bool keepChecking = true;										\
		if ( (((bool)(exp)) == false) && keepChecking )							\
		{																		\
			System::Log("[ASSERT]  %s", description);							\
			System::Log("          - %s (Line: %u)", __FILE__, __LINE__);		\
			if ( R5::Thread::AssertWindow( description,	__LINE__, __FILE__, keepChecking) ) { __asm { int 3 } }	\
		}																		\
	}
	#define WARNING(text)	ASSERT(false, text)
	#define DEBUG(text)		R5::Thread::MessageWindow(text);

#else

	#define ASSERT( exp, description )
	#define WARNING(text)	R5::System::Log("[WARNING] %s", text);
	#define DEBUG(text)

#endif