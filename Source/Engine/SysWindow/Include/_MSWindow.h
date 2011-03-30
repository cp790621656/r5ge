#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Main window class -- creates the window, manages events
//============================================================================================================

namespace R5
{
class SysWindow : public IWindow
{
public:

	// Event codes are here primarily to avoid using WM_* macros
	struct Event
	{
		enum Type
		{
			Move					= 0x0003,
			Resize					= 0x0005,
			Activate				= 0x0006,
			Paint					= 0x000F,
			Close					= 0x0010,
			KeyDown					= 0x0100,
			KeyUp					= 0x0101,
			Char					= 0x0102,
			SysKeyDown				= 0x0104,
			SysKeyUp				= 0x0105,
			SysChar					= 0x0106,
			MouseMove				= 0x0200,
			LeftButtonDown			= 0x0201,
			LeftButtonUp			= 0x0202,
			DoubleClick				= 0x0203,
			RightButtonDown			= 0x0204,
			RightButtonUp			= 0x0205,
			MiddleButtonDown		= 0x0207,
			MiddleButtonUp			= 0x0208,
			XButtonDown				= 0x020B,
			XButtonUp				= 0x020C,
			MouseWheel				= 0x020A
		};
	};

	// Message callback function, declared as a friend so it can access the class' data
	friend long GlobalEventHandler(HWND hWnd, uint eventId, uint first, uint second);

protected:

	HINSTANCE			mHInstance;			// ModelInstance handle
	HWND				mHWnd;				// Window handle
	HWND				mHParent;			// Parent window handle
	HDC					mHDC;				// Device context handle
	Thread::IDType		mWindowThread;		// ID of the thread in which the window was created

	String				mTitle;				// Window title
	uint				mStyle;				// Window style
	uint				mPrevStyle;			// Previous window style, before window was closed (for serialization)
	bool				mIsMinimized;		// Whether the window is minimized
	bool				mIgnoreResize;		// Whether to ignore resize messages (windows hack)
	Vector2i			mPos;				// Window position
	Vector2i			mSize;				// Window dimensions
	Vector2i			mFsSize;			// Full screen size
	IEventReceiver*		mHandler;			// Event notifier
	IGraphics*			mGraphics;			// Associated graphics controller
	Thread::Lockable	mLock;

public:

	SysWindow();
	virtual ~SysWindow();

protected:

	void Lock()		const	{ mLock.Lock();	}
	void Unlock()	const	{ mLock.Unlock();	}

	// Registers the window class
	void _Register(  USHORT iconID, USHORT cursorID );

	// Create / destroy window handles
	bool _CreateHandles();
	void _ReleaseHandles();

	// Create / destroy the rendering context
	virtual bool _CreateContext() { return true; }
	virtual void _ReleaseContext() {}

	// INTERNAL: Changes position, size and/or goes into full screen mode
	bool _Set (const Vector2i& pos, const Vector2i& size, uint style);

public:

	// Custom functions
	HDC		GetDC		() { return mHDC; }
	HWND	GetHandle	() { return mHWnd; }
	void	Post (UINT uMsg, UINT wParam, UINT lParam);

public:

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

	// IWindow functions
	virtual void SetTitle		( const String& title );
	virtual void SetEventHandler( IEventReceiver* ptr ) { mHandler  = ptr; }
	virtual bool SetGraphics	( IGraphics*	 ptr );
	virtual bool SetPosition	(const Vector2i& pos);
	virtual bool SetSize		(const Vector2i& size);
	virtual bool SetStyle		(uint style);
	virtual void SetFocus		();

	virtual bool		IsValid()		const	{ return mHWnd != 0; }
	virtual String		GetTitle()		const	{ return mTitle; }
	virtual Vector2i	GetPosition()	const	{ return (mStyle == Style::FullScreen) ? Vector2i(0, 0) : mPos; }
	virtual Vector2i	GetSize()		const	{ return (mStyle == Style::FullScreen) ? mFsSize : mSize; }
	virtual uint		GetStyle()		const	{ return (mStyle == Style::Undefined) ? mPrevStyle : mStyle; }
	virtual bool		IsMinimized()	const	{ return mIsMinimized; }
	virtual void		ShowCursor(bool show);
	virtual void		Close();
	virtual bool		Update();
	virtual void		BeginFrame()	{ mLock.Lock();   }
	virtual void		EndFrame()		{ mLock.Unlock(); }

	// Retrieves a string from the clipboard
	virtual String GetClipboardText() const;

	// Sets the system clipboard text
	virtual void SetClipboardText (const String& text);

	// Serialization
	virtual bool SerializeFrom	(const TreeNode& root);
	virtual bool SerializeTo	(TreeNode& root) const;
};
};	// namespace R5