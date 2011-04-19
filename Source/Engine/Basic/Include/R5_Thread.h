#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
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

#ifdef _LINUX
	class Lockable
	{
	protected:
		mutable pthread_spinlock_t mLock;
		
	public:
		Lockable() { pthread_spin_init(&mLock, 0); }
		~Lockable() { pthread_spin_destroy(&mLock);}

	public:
		inline void Lock()		const	{ pthread_spin_lock(&mLock); }
		inline void Unlock()	const	{ pthread_spin_unlock(&mLock); }
		inline bool IsLocked()	const	
		{
			int retval = pthread_spin_trylock(&mLock);
			if (retval == EDEADLK || retval == EBUSY) return true;
			pthread_spin_unlock(&mLock);
			return false;
		}
	};

#else

	class Lockable
	{
	protected:
		mutable ValType	mLock;
		
	public:
		Lockable()	: mLock(0) {}
		
	public:
		void Lock()		const	{ WaitFor(mLock); }
		bool IsLocked()	const	{ return mLock != 0; }
		void Unlock()	const	{ mLock = 0; }
	};

#endif
};
