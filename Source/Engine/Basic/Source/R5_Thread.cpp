#include "../Include/_All.h"

//============================================================================================================
#ifdef _WINDOWS
//============================================================================================================

#include <windows.h>
#pragma comment(lib, "winmm.lib")

using namespace R5;

//------------------------------------------------------------------------------------------------------------

void Thread::Increment	(ValType& val)		{ ::InterlockedIncrement( &val ); }
void Thread::Decrement	(ValType& val)		{ ::InterlockedDecrement( &val ); }
void Thread::WaitFor	(ValType& val)		{ while (::InterlockedCompareExchange(&val, 1, 0)) ::Sleep(0); }

//------------------------------------------------------------------------------------------------------------

void Thread::Sleep(ulong ms)
{
	::Sleep(ms);
}

//------------------------------------------------------------------------------------------------------------

void Thread::ImproveTimerFrequency(bool improve)
{
	static bool set = false;
	if (set != improve)
	{
		if (set = improve) ::timeBeginPeriod(1);
		else ::timeEndPeriod(1);
	}
}

//------------------------------------------------------------------------------------------------------------

void Thread::MessageWindow(const char *format, ...)
{
	char *args, *text;
	va_start(args, format);
	text = new char[_vscprintf(format, args) + 1];
	vsprintf(text, format, args);
	va_end(args);
	::MessageBox(NULL, text, "Information", 0);
	delete [] text;
}

//------------------------------------------------------------------------------------------------------------

bool Thread::AssertWindow( const char* description, int line, const char* filename, bool& keepChecking )
{
	char* format = "File: '%s'\nLine: %u\n\n%s\n\nBreak into code?";

	uint size = _scprintf(format, filename, line, description) + 1;
	char* text = new char[size+1];
	
	sprintf(text, format, filename, line, description);

	int result = ::MessageBox( NULL, text, "Assert Failed!", MB_YESNOCANCEL | MB_ICONERROR | MB_TASKMODAL );

	delete [] text;

	if ( result == IDYES ) return true;
	if ( result == IDNO  ) return false;
	
	keepChecking = false;
	return false;
}

//------------------------------------------------------------------------------------------------------------

void* Thread::Create(DelegateFunction fnc, void* argument)
{
	return ::CreateThread(0, 0, fnc, argument, 0, 0);
}

//------------------------------------------------------------------------------------------------------------

void Thread::Terminate(void* handle)
{
	if (handle == 0) ::PostQuitMessage(0);
	else ::TerminateThread((HANDLE)handle, 0);
}

//------------------------------------------------------------------------------------------------------------

Thread::IDType Thread::GetID()
{
	return (Thread::IDType)::GetCurrentThreadId();
}

//============================================================================================================
#elif defined _MACOS
//============================================================================================================

#include <libkern/OSAtomic.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#import <Foundation/NSString.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSAlert.h>

using namespace R5;

//------------------------------------------------------------------------------------------------------------

void Thread::Increment	(ValType& val)		{ ::OSAtomicIncrement32( &val ); }
void Thread::Decrement	(ValType& val)		{ ::OSAtomicDecrement32( &val ); }
void Thread::WaitFor	(ValType& val)		{ ::OSSpinLockLock     ( &val ); }
void Thread::Sleep		(ulong ms)	{ ::usleep(ms * 1000UL); }

//------------------------------------------------------------------------------------------------------------

void Thread::ImproveTimerFrequency(bool improve)
{
}

//------------------------------------------------------------------------------------------------------------

void* Thread::Create(DelegateFunction fnc, void* argument)
{
	pthread_t id;
	pthread_create(&id, 0, fnc, argument);
	return (void*)id;
}

//------------------------------------------------------------------------------------------------------------

void Thread::Terminate (void* handle)
{
	if (handle == 0) pthread_exit(0);
	else pthread_cancel((pthread_t)handle);
}

//------------------------------------------------------------------------------------------------------------

Thread::IDType Thread::GetID()
{
	return (Thread::IDType)pthread_self();
}

//------------------------------------------------------------------------------------------------------------

void Thread::MessageWindow(const char *format, ...)
{
	va_list args;
	char text[512] = {0};
	va_start(args, format);
	vsprintf(text, format, args);
	va_end(args);
	
	// Managed Obj-C should be wrapped into an auto-release pool so it doesn't leak memory
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
	NSString* nsText = [NSString stringWithUTF8String:text];
	NSAlert* alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"OK"];
	[alert setMessageText:@"Information"];
	[alert setInformativeText:nsText];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert runModal];
	[alert release];
	[pool drain];
}

//------------------------------------------------------------------------------------------------------------

bool Thread::AssertWindow( const char* description, int line, const char* filename, bool& keepChecking )
{
	const char* format = "File: '%s'\nLine: %u\n\n%s\n\nBreak into code?";
	char text[1024]; spin lock
	
	sprintf(text, format, filename, line, description);

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
	NSString* nsText = [NSString stringWithUTF8String:text];
	NSAlert* alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Break"];
	[alert addButtonWithTitle:@"Continue"];
	[alert addButtonWithTitle:@"Ignore"];
	[alert setMessageText:@"Assert!"];
	[alert setInformativeText:nsText];
	[alert setAlertStyle:NSWarningAlertStyle];
	int result = [alert runModal];
	[alert release];
	[pool drain];

	if ( result == NSAlertFirstButtonReturn  ) return true;
	if ( result == NSAlertSecondButtonReturn ) return false;
	
	keepChecking = false;
	return false;
}

//============================================================================================================
#else // Linux
//============================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

using namespace R5;

//------------------------------------------------------------------------------------------------------------

inline Thread::ValType InterlockedCompareExchange (volatile Thread::ValType* ptr,
															Thread::ValType  cmp,
															Thread::ValType  set)
{
	return __sync_val_compare_and_swap(ptr, cmp, set);

	//Thread::ValType prev;
	//__asm__ __volatile__("lock; cmpxchgl %1,%2"
	//				   : "=a" (prev)
	//				   : "q" (set), "m" (*ptr), "0" (cmp)
	//				   : "memory");
	//return prev;
}

//------------------------------------------------------------------------------------------------------------

void Thread::Increment	(ValType& val)	{ __sync_add_and_fetch( &val, 1 ); }
void Thread::Decrement	(ValType& val)	{ __sync_sub_and_fetch( &val, 1 ); }
void Thread::WaitFor	(ValType& val)	{ while (::InterlockedCompareExchange(&val, 1, 0)) ::usleep(0); }
void Thread::Sleep		(ulong ms)		{ ::usleep(ms * 1000UL); }

//------------------------------------------------------------------------------------------------------------

void Thread::ImproveTimerFrequency(bool improve)
{
}

//------------------------------------------------------------------------------------------------------------

void* Thread::Create(DelegateFunction fnc, void* argument)
{
	pthread_t id;
	pthread_create(&id, 0, fnc, argument);
	return (void*)id;
}

//------------------------------------------------------------------------------------------------------------

void Thread::Terminate (void* handle)
{
	if (handle == 0) pthread_exit(0);
	else pthread_cancel((pthread_t)handle);
}

//------------------------------------------------------------------------------------------------------------

Thread::IDType Thread::GetID()
{
	return (Thread::IDType)pthread_self();
}

//------------------------------------------------------------------------------------------------------------

void Thread::MessageWindow(const char *format, ...) {}
bool Thread::AssertWindow(const char* description, int line, const char* filename, bool& keepChecking) 
{
	fprintf(stderr, "Assert Failed: %s at %d in %s\n", description, line, filename);
	return true;
}

#endif
