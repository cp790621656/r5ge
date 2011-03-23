#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2008 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Objective-C Class prototypes, declared here in order to avoid including <Cocoa/Cocoa.h> in the header file
//============================================================================================================

#ifdef _MACOS
@class NSAutoreleasePool, NSOpenGLContext, MyView, MyWindow;

//============================================================================================================
// Cocoa managed memory pool
//============================================================================================================

namespace R5
{
class ManagedMemory : public Thread::Lockable
{
	NSAutoreleasePool*	mPool;

public:

	ManagedMemory();
	~ManagedMemory();
};

//============================================================================================================
// Cocoa window implementation
//============================================================================================================

class SysWindow : public IWindow
{
protected:

	String				mTitle;
	ManagedMemory		mPool;
	MyWindow*			mWin;
	MyView*				mView;
	bool				mFullScreen;

	Vector2i			mPos;
	Vector2i			mSize;
	Vector2i			mFsSize;
	bool				mForceUpdate;
	IGraphics*			mGraphics;
	uint				mStyle;
	uint				mPrevStyle;
	Thread::Lockable	mLock;

protected:

	void Lock()		const	{ mLock.Lock();	}
	void Unlock()	const	{ mLock.Unlock();	}

	// INTERNAL: Enters the full screen mode
	bool _EnterFullScreen (bool enter);

	// Sets the window's position and size
	bool _Set (const Vector2i& pos, const Vector2i& size);

	// Overwritten by the OpenGL window implementation
	virtual bool _CreateContext() { return true; }
	virtual void _UpdateContext() {}
	virtual void _ReleaseContext() {}

public:

	SysWindow ();
	virtual ~SysWindow();

	// Creates a window of specified name, style, width, and height
	virtual bool Create(const String&	title,
						short			x			= 0,
						short			y			= 0,
						ushort			width		= 1024,
						ushort			height		= 768,
						uint			style		= Style::Normal,
						ushort			iconID		= 0,
						ushort			cursorID	= 0,
						void*			pParent		= 0);

	virtual void SetTitle		( const String& title );
	virtual void SetEventHandler( IEventReceiver* ptr );
	virtual bool SetGraphics	( IGraphics*	 ptr );
	virtual void SetFocus		();
	virtual bool SetStyle		(uint style);
	virtual bool SetPosition	(const Vector2i& pos);
	virtual bool SetSize		(const Vector2i& size);

	// Various functions
	virtual bool		IsValid()		const;
	virtual String		GetTitle()		const	{ return mTitle; }
	virtual Vector2i	GetPosition()	const	{ return (mStyle == Style::FullScreen) ? Vector2i(0, 0) : mPos; }
	virtual Vector2i	GetSize()		const	{ return (mStyle == Style::FullScreen) ? mFsSize : mSize; }
	virtual uint		GetStyle()		const	{ return mStyle; }
	virtual bool		IsMinimized()	const;
	virtual void		ShowCursor (bool show);
	virtual void		Close();
	virtual bool		Update();
	virtual void		BeginFrame()	{ mLock.Lock();   }
	virtual void		EndFrame()		{ mLock.Unlock(); }

	// Retrieves a string from the clipboard
	virtual String GetClipboardText() const;

	// Sets the system clipboard text
	virtual void SetClipboardText (const String& text);

	// Serialization
	virtual bool SerializeFrom (const TreeNode& root);
	virtual bool SerializeTo (TreeNode& root) const;
};
}; // namespace R5

#endif // _MACOS