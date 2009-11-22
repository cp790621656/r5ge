#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

namespace Thread
{
#ifdef _WINDOWS
	typedef ulong ReturnType;
	typedef ReturnType (__stdcall *DelegateFunction)(void*);
	typedef long ValType;
	typedef long IDType;
#else
	typedef void* ReturnType;
	typedef ReturnType (*DelegateFunction)(void*);
	typedef int ValType;
	typedef void* IDType;
#endif

	void	Increment(ValType& val);
	void	Decrement(ValType& val);
	void	Sleep(ulong ms);											// System's basic sleep function
	void	WaitFor(ValType& val);										// InterlockedCompareExchange()
	void	ImproveTimerFrequency(bool improve);						// Ensures that timeGetTime() frequency is 1ms
	void	MessageWindow(const char *sFormat, ...);					// Shows a message box on the screen
	bool	AssertWindow(const char* description, int line,				// Shows an assert window with options to abort, retry and ignore
						 const char* filename, bool& keepChecking );
	void*	Create (DelegateFunction fnc, void* argument);
	void	Terminate (void* handle = 0);

	IDType	GetID();

// PThread-using implementation, for reference:
//#ifdef _LINUX
//	
//	class Lockable
//	{
//	protected:
//		mutable pthread_mutex_t	mLock;
//		
//	public:
//		Lockable() { pthread_mutex_init(&mLock, 0); }
//		
//	public:
//		inline void Lock()		const	{ pthread_mutex_lock(&mLock); }
//		inline void Unlock()	const	{ pthread_mutex_unlock(&mLock); }
//	};
//
//#else

	class Lockable
	{
	protected:
		mutable ValType	mLock;
		
	public:
		Lockable()	: mLock(0) {}
		
	public:
		inline void Lock()		const	{ WaitFor(mLock); }
		inline void Unlock()	const	{ mLock = 0; }
	};

//#endif
};