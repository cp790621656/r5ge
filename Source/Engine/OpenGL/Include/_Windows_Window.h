#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// OpenGL Window creation
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _WINDOWS

class GLWindow : public SysWindow
{
private:

	Thread::IDType	mGraphicsThread;	// ID of the thread in which the graphics system was instantiated in
	void*			mHRC;				// Rendering context
	uint			mMSAA;

public:

	GLWindow (uint msaa = 0) : mGraphicsThread(0), mHRC(0), mMSAA(msaa) {}

protected:

	virtual bool _CreateContext();
	virtual void _ReleaseContext();
	virtual void BeginFrame();
	virtual void EndFrame();
};

#endif