#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c)2008 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// OpenGL Window creation
//============================================================================================================

#ifdef _MACOS

class GLWindow : public SysWindow
{
private:

	void*				mGraphicsThread;
	NSOpenGLContext*	mContext;

public:

	GLWindow (uint msaa = 0) : mGraphicsThread(0), mContext(0) {}

protected:

	virtual bool _CreateContext();
	virtual void _UpdateContext();
	virtual void _ReleaseContext();
	virtual void BeginFrame();
	virtual void EndFrame();
};

#endif