#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================

#ifndef _BASIC_TYPES_DEFINE_H
#define _BASIC_TYPES_DEFINE_H

#ifdef _MSC_VER

	#ifndef _WINDOWS
	#define _WINDOWS
	#endif

	#ifndef R5_MAIN_FUNCTION
	#define R5_MAIN_FUNCTION int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, char* sCmdLine, int iShow)
	#endif

	#ifndef R5_THREAD_FUNCTION
	#define R5_THREAD_FUNCTION(name, ptr)	ulong __stdcall name(void* ptr)
	#endif

	#ifndef int32
	#define int32 __int32
	#endif

	#ifndef uint32
	#define uint32 unsigned __int32
	#endif

#else

	#ifdef __APPLE__
		#ifndef _MACOS
		#define _MACOS
		#endif
	#elif __linux__
		#ifndef _LINUX
		#define _LINUX
		#endif
	#endif

	#ifndef __stdcall
	#define __stdcall __attribute__((stdcall))
	#endif

	#ifndef R5_MAIN_FUNCTION
	#define R5_MAIN_FUNCTION int main (int argc, char* argv[])
	#endif

	#ifndef R5_THREAD_FUNCTION
	#define R5_THREAD_FUNCTION(name, ptr)	void* name(void* ptr)
	#endif

	#ifndef int32
	#define int32 __int32_t
	#endif

	#ifndef uint32
	#define uint32 __uint32_t
	#endif

#endif

namespace R5
{
	typedef void* VoidPtr;
	typedef unsigned char* BytePtr;
	typedef const unsigned char* ConstBytePtr;
};

#ifndef FOREACH
#define FOREACH(var, array) for (uint var = 0; var < array.GetSize(); ++var)
#endif

#ifndef _R5_MATH_DEFINES
#define _R5_MATH_DEFINES

	#define SQRT	sqrtf
	#define SIN		sinf
	#define ASIN	asinf
	#define COS		cosf
	#define ACOS	acosf
	#define TAN		tanf
	#define ATAN	atanf
	#define ATAN2	atan2f
	#define LOG		logf

#endif

typedef unsigned char	byte;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;

#ifdef _WINDOWS

 // Disables "macro redefinition" warning
 #pragma warning(disable: 4005)

 #define _WIN32_WINNT	0x0502

 // Common "windows.h" definitions -- out here so "windows.h" doesn't need to be included in the header file
 #ifndef _WINDOWS_
  typedef struct HDC__*			HDC;
  typedef struct HWND__*		HWND;
  typedef struct HGLRC__*		HGLRC;
  typedef struct HINSTANCE__*	HINSTANCE;

  #define WINGDIAPI		__declspec(dllimport)
  #define APIENTRY		__stdcall
  #define WINAPI		__stdcall
  #define CALLBACK		__stdcall
  #define IMPORT		__declspec(dllimport)

  typedef unsigned char		BYTE;
  typedef unsigned short	USHORT;
  typedef unsigned int		UINT;
  typedef long				LONG;
  typedef unsigned long		ULONG;
  typedef unsigned long		DWORD;
 #else
  // Windows.h has a bad habit of #define-renaming "GetMessage" into "GetMessageA"
  #undef GetMessage
 #endif

#endif // _WINDOWS

template <typename Real>
void Swap(Real& val0, Real& val1)
{
	Real temp ( val0 );
	val0 = val1;
	val1 = temp;
}

#define INVALID_VAL (uint)-1

#endif // _BASIC_TYPES_DEFINE_H
