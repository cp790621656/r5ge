#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

#ifndef _BASIC_INCLUDE_H
#define _BASIC_INCLUDE_H

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

//#define R5_MEMORY_TEST

#include <string.h>
#include <stdio.h>

// Function delegate functionality by Jody Hagins -- see the header file for more information
#include "../FastDelegate/FastDelegate.h"
#include "../FastDelegate/FastDelegateBind.h"

// Convenience syntax
using namespace fastdelegate;
using fastdelegate::bind;

// Definitions
#include "R5_Defines.h"				// Basic system-dependent preprocessor declarations
#include "R5_Assert.h"				// Assertion functionality for debugging

//#if defined(_LINUX)
//#include <pthread.h>
//#endif

// Fluid Studios memory manager to test for possible memory leaks -- see the header file for more information
#ifdef R5_MEMORY_TEST
#include "../Memmgr/mmgr.h"
#endif

namespace R5
{
	#include "R5_Flags.h"			// A very simple embeddable flag class
	#include "R5_Time.h"			// Time tracking
	#include "R5_Thread.h"			// Multithreading related functions
	#include "R5_String.h"			// High performance string class
	#include "R5_System.h"			// System-specific functions
	#include "R5_Memory.h"			// Basic memory buffer
	#include "R5_BaseArray.h"		// Unfinished array template used by Array and PointerArray
	#include "R5_Array.h"			// Highly optimized dynamic array template, std::vector replacement
	#include "R5_PointerArray.h"	// Same as an array, but for pointers -- automatically deletes them
	#include "R5_ResourceArray.h"	// Pointer array for named resources -- used by manager classes
	#include "R5_LinkedList.h"		// Linked list template (FIFO)
	#include "R5_Hash.h"			// uint-based hash template
	#include "R5_PointerHash.h"		// Hash meant to store pointers -- automatically deletes them
	#include "R5_Keys.h"			// Key map
	#include "R5_Random.h"			// Cross-platform pseudo-random number generator
};

#endif