#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// OpenGL Window creation
//============================================================================================================

#ifdef _WINDOWS

class GLWindow : public SysWindow
{
private:

	Thread::IDType	mGraphicsThread;	// ID of the thread in which the graphics system was instantiated in
	void*			mHRC;				// Rendering context

public:

	GLWindow() : mGraphicsThread(0), mHRC(0) {}

protected:

	virtual bool _CreateContext();
	virtual void _ReleaseContext();
	virtual void BeginFrame();
	virtual void EndFrame();
};

#endif