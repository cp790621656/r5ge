//==============================================================================
//                   Copyright (c) 2011 Eugene Gorodinsky
//==============================================================================
//            X11 SysWindow class. Same as other SysWindow classes.
//==============================================================================


// Define some types so as not to include the whole glx.h and xlib headers
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef XID Window;
typedef XID Cursor;
typedef struct __GLXcontextRec *GLXContext;

namespace R5
{
class SysWindow : public IWindow
{
private:
	Thread::IDType		mWindowThread;
	Thread::Lockable	mLock;

	Display				*mDisplay;
	Window				mWin;
	Cursor				mInvisibleCursor; // Invisible cursor for ShowCursor(false)
	GLXContext			mGLXContext;

	uint					mStyle;
	uint					mPrevStyle;			// For serialization
	IEventReceiver*	mHandler;			// Event notifier
	IGraphics*			mGraphics;			// Associated graphics controller
	String				mTitle;				// Window title
	Vector2i				mSize;				// Window size
	Vector2i				mPos;					// Position
	bool					mIsMinimized;
	bool					mIgnoreResize;

	String				mClipboard;

public:
	SysWindow();
	virtual ~SysWindow();

protected:

	void Lock()		const	{ mLock.Lock();	}
	void Unlock()	const	{ mLock.Unlock();	}

	// Create / destroy the rendering context
	virtual bool _CreateContext();
	virtual void _ReleaseContext();

public:
	// Creates a window of specified name, style, width, and height
	virtual bool Create(	const String&	title,
								short				x			= 0,
								short				y			= 0,
								ushort			width		= 1024,
								ushort			height	= 768,
								uint				style		= Style::Normal,
								ushort			iconID	= 0,
								ushort			cursorID	= 0,
								void*				hParent	= 0);

	virtual void SetTitle			(const String& title);
	virtual void SetEventHandler	(IEventReceiver* ptr) { mHandler  = ptr; }
	virtual bool SetGraphics		(IGraphics* ptr);
	virtual bool SetPosition		(const Vector2i& pos);
	virtual bool SetSize				(const Vector2i& size);
	virtual bool SetStyle			(uint style);
	virtual void SetFocus			();

	// Various functions
	virtual bool		IsValid()		const { return mWin != 0; }
	virtual String		GetTitle()		const	{ return mTitle; }
	virtual Vector2i	GetPosition()	const { return mPos; }
	virtual Vector2i	GetSize()		const { return mSize; }
	virtual uint		GetStyle()		const { return (mStyle == Style::Undefined) ? mPrevStyle : mStyle; }
	virtual bool		IsMinimized()	const { return mIsMinimized; }
	virtual void		ShowCursor(bool show);
	virtual void		Close();
	virtual bool		Update();
	virtual void		BeginFrame();
	virtual void		EndFrame();

	// Retrieves a string from the clipboard
	virtual String GetClipboardText() const;

	// Sets the system clipboard text
	virtual void SetClipboardText (const String& text);

	// Serialization
	virtual bool SerializeFrom	(const TreeNode& root);
	virtual bool SerializeTo	(TreeNode& root) const;
};
};	// namespace R5
