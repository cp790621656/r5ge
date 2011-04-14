#include "../Include/_All.h"

#ifdef _LINUX

#include <unistd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>

using namespace R5;

Atom wmDeleteMessage;

//============================================================================================================
// Converts X11 key codes to R5 key codes
//============================================================================================================

inline byte ConvertKeyCode (byte key)
{
	// It appears that simple, although needs to be tested on other keyboards
	return key - 9;
}

//============================================================================================================
// Set up the default values for everything
//============================================================================================================

SysWindow::SysWindow() :
	mDisplay		(0),
	mWin			(0),
	mWindowThread	(0),
	mStyle			(Style::Undefined),
	mPrevStyle		(Style::Undefined),
	mIsMinimized	(false),
	mIgnoreResize	(false),
	mPos			(0),
	mSize			(0),
	mHandler		(0),
	mGraphics		(0)
{
}

//============================================================================================================

SysWindow::~SysWindow()
{
	Close();
	::XCloseDisplay(mDisplay);
}

//============================================================================================================

bool SysWindow::_CreateContext()
{
	::glXMakeCurrent(mDisplay, mWin, mGLXContext);
	return (mGraphics == 0) || mGraphics->Init();
}

//============================================================================================================

void SysWindow::_ReleaseContext()
{
	::glXMakeCurrent(mDisplay, None, NULL);
}

//============================================================================================================
// Creates a window of specified name, width, and height
// hParent, iconID and cursorID are ignored, if style is Style::Child, the program aborts
//============================================================================================================

bool SysWindow::Create(
	const	String& title,
	short	x,
	short	y,
	ushort	width,
	ushort	height,
	uint	style,
	ushort	iconID,
	ushort	cursorID,
	void*	hParent)
{
	Lock();

	if (!mDisplay)
	{
		mDisplay = ::XOpenDisplay(NULL);
	}
	
	bool retval = false;
	
	if (!mWin)
	{
		Window root;
		root = ::XDefaultRootWindow(mDisplay);
		int screen = ::XDefaultScreen(mDisplay);

		static int visual_attribs[] =
		{
		   GLX_X_RENDERABLE    , True,
		   GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		   GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		   GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		   GLX_RED_SIZE        , 8,
		   GLX_GREEN_SIZE      , 8,
		   GLX_BLUE_SIZE       , 8,
		   GLX_ALPHA_SIZE      , 8,
		   GLX_DEPTH_SIZE      , 24,
		   GLX_STENCIL_SIZE    , 8,
		   GLX_DOUBLEBUFFER    , True,
		   GLX_SAMPLE_BUFFERS  , 1,
		   GLX_SAMPLES         , 4,
		   None
		};
		int nelements;

		GLXFBConfig *fbc = ::glXChooseFBConfig(mDisplay, screen, visual_attribs, &nelements);

		XVisualInfo *vi = ::glXGetVisualFromFBConfig(mDisplay, fbc[0]);

		mGLXContext = ::glXCreateContext(mDisplay, vi, NULL, GL_TRUE);

		XSetWindowAttributes swa;
		swa.colormap	=	::XCreateColormap(mDisplay, root, vi->visual, AllocNone);
		swa.event_mask 	=	PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
							KeyPressMask | KeyReleaseMask | StructureNotifyMask;

		swa.background_pixel = 	WhitePixel(mDisplay, screen);

		mWin = ::XCreateWindow(	mDisplay, root, x, y, width, height,
								0, vi->depth, InputOutput, vi->visual,
								CWColormap | CWEventMask | CWBackPixel, &swa );
		SetTitle(title);

		mSize.Set(width, height);
		mPos.Set(x, y);
	
		SetStyle(style);
	
		if (style != IWindow::Style::Hidden)
			::XMapWindow(mDisplay, mWin);

		// Create an invisible cursor for ShowCursor(false)
		XColor black;
		static char pixel = 0;
		black.red = black.green = black.blue = 0;

		Pixmap bitmap = XCreateBitmapFromData(mDisplay, mWin, &pixel, 1, 1);
		mInvisibleCursor = XCreatePixmapCursor(mDisplay, bitmap, bitmap, &black, &black, 0, 0);

		// WM_DELETE_WINDOW notification is received when a window is being closed. Register interest in it.
		wmDeleteMessage = ::XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
		::XSetWMProtocols(mDisplay, mWin, &wmDeleteMessage, 1);

		mWindowThread = Thread::GetID();
		
		retval = _CreateContext();
	}
	
	Unlock();
	return retval;	
}

//==========================================================================================================
// Processes events and returns whether the application needs to exit
//==========================================================================================================

bool SysWindow::Update()
{
	static bool KeyRepeat = false;
	static Vector2i mousePos;

	if (mWin == 0 || mDisplay == NULL) return false;

	if (mWindowThread == Thread::GetID())
	{
		while (XPending(mDisplay))
		{
      		XEvent xev;
			::XNextEvent(mDisplay, &xev);

			switch (xev.type)
			{
				case KeyPress:
				{
					KeySym xk = XLookupKeysym(&xev.xkey, 0);

					if (xk < 128)
					{
						mHandler->OnChar((char)xk);
					}

					if (!KeyRepeat)
					{
						mHandler->OnKeyPress(Vector2i(xev.xkey.x, xev.xkey.y), ConvertKeyCode(xk), true);
					}
				}
				break;

				case KeyRelease:
				{
					if (XEventsQueued(mDisplay, QueuedAfterReading))
					{
						XEvent nev;
						XPeekEvent(mDisplay, &nev);

						if (nev.type == KeyPress && 
							nev.xkey.time == xev.xkey.time &&
							nev.xkey.keycode == xev.xkey.keycode)
						{
							KeyRepeat = true;
							break;
						}
					}

					// If we got to here, then this is not a key repeat
					KeyRepeat = false;
					mHandler->OnKeyPress(Vector2i(xev.xkey.x, xev.xkey.y), ConvertKeyCode(XLookupKeysym(&xev.xkey, 0)), false);
				}
				break;

				case ButtonPress:
				{
					switch (xev.xbutton.button)
					{
						case Button1: 
							mHandler->OnKeyPress(Vector2i(xev.xbutton.x, xev.xbutton.y), Key::MouseLeft, true); 
							break;
					
						case Button2:
							mHandler->OnKeyPress(Vector2i(xev.xbutton.x, xev.xbutton.y), Key::MouseMiddle, true); 
							break;
					
						case Button3: 
							mHandler->OnKeyPress(Vector2i(xev.xbutton.x, xev.xbutton.y), Key::MouseRight, true); 
							break;
					
						case Button4:
							mHandler->OnScroll(Vector2i(xev.xbutton.x, xev.xbutton.y), 1.0f); 
							break;
					
						case Button5: 
							mHandler->OnScroll(Vector2i(xev.xbutton.x, xev.xbutton.y), -1.0f); 
							break;	 
					}
				}
				break;

				case ButtonRelease:
				{
					switch (xev.xbutton.button)
					{
						case Button1: 
							mHandler->OnKeyPress(Vector2i(xev.xbutton.x, xev.xbutton.y), Key::MouseLeft, false);
							break;
					
						case Button2:
							mHandler->OnKeyPress(Vector2i(xev.xbutton.x, xev.xbutton.y), Key::MouseMiddle, false);
							break;
					
						case Button3: 
							mHandler->OnKeyPress(Vector2i(xev.xbutton.x, xev.xbutton.y), Key::MouseRight, false);
							break;
					}
				}
				break;

				case MotionNotify:
				{
					static Vector2i prevPos(0,0);
					static Vector2i mousePos(xev.xmotion.x, xev.xmotion.y);
					mHandler->OnMouseMove(mousePos, mousePos - prevPos);
					prevPos = mousePos;
				}
				break;

				case ClientMessage:
					if (xev.xclient.data.l[0] == (long)wmDeleteMessage)
						Close();

				case MapNotify:
					mIsMinimized = false;
					break;

				case UnmapNotify:
					mIsMinimized = true;
					break;

				case ConfigureNotify:
				{
					mPos.Set(xev.xconfigure.x, xev.xconfigure.y);
					mSize.Set(xev.xconfigure.width, xev.xconfigure.height);
					mHandler->OnResize(mSize);
				}
				break;

				case SelectionRequest:
				{
					XSelectionEvent select;
					select.type = SelectionNotify;
					select.display = xev.xselection.display;
					select.requestor = xev.xselection.requestor;
					select.selection = xev.xselection.selection;
					select.target = xev.xselection.target;
					select.time = xev.xselection.time;
					select.property = None;

					Atom targets = XInternAtom (mDisplay, "TARGETS", False);

					if (xev.xselection.target == targets)
					{
						unsigned int target_list[] = {XA_STRING};
						XChangeProperty(xev.xselection.display, 
										xev.xselection.requestor, 
										xev.xselection.property, 
										XA_ATOM, 
										32, 
										PropModeReplace, 
										(unsigned char *)target_list,
						        		sizeof(target_list)/sizeof(int));
					}
					else if (xev.xselection.target == XA_STRING)
					{
						XChangeProperty(xev.xselection.display, 
										xev.xselection.requestor, 
										xev.xselection.property, 
										XA_STRING, 
										8, 
										PropModeReplace, 
										(unsigned char *)mClipboard.GetBuffer(), 
										mClipboard.GetLength());
					}
					select.property = xev.xselection.property;

					XSendEvent(	xev.xselection.display, 
								xev.xselection.requestor, 
								False, 
								0, 
								(XEvent*)&select);
				}
			}
      }
		
		if (mIsMinimized) ::usleep(5000);
	}
	return mStyle != Style::Undefined;
}

//==========================================================================================================
// Set the window's title
//==========================================================================================================

void SysWindow::SetTitle (const String& title)
{
	if (mWin != 0 && mDisplay != NULL)
	{
		::XStoreName(mDisplay, mWin, title);
		mTitle = title;	
	}
}

//==========================================================================================================
// Set the window's size
//==========================================================================================================

bool SysWindow::SetSize(const Vector2i& size)
{
	if (mWin != 0 && mDisplay != NULL)
	{
		::XResizeWindow(mDisplay, mWin, size.x, size.y);
		mSize = size;
	}
	return true;
}

//==========================================================================================================
// Set the input focus to the window
//==========================================================================================================

void SysWindow::SetFocus()
{
	if (mWin != 0 && mDisplay != NULL)
	{
		::XSetInputFocus(mDisplay, mWin, RevertToParent, CurrentTime);
	}
}

//==========================================================================================================
// Set window's position
//==========================================================================================================

bool SysWindow::SetPosition(const Vector2i& pos)
{
	if (mWin != 0 && mDisplay != NULL)
	{
		::XMoveWindow(mDisplay, mWin, pos.x, pos.y);
		mPos = pos;
	}
	return true;
}

//==========================================================================================================
// Closes the window
//==========================================================================================================

void SysWindow::Close()
{
	if (mStyle != Style::Undefined && mWin != 0 && mDisplay != NULL)
	{
		mPrevStyle	= mStyle;
		mStyle		= Style::Undefined;
		mHandler	= 0;

		::XFreeCursor(mDisplay, mInvisibleCursor);
		_ReleaseContext();
		::XDestroyWindow(mDisplay, mWin);
		mWin = 0;
	}
}

//==========================================================================================================
// Shows or hides the mouse cursor
//==========================================================================================================

void SysWindow::ShowCursor(bool show)
{
	static bool hidden = false;

	if (show && hidden)	
	{
		::XUndefineCursor(mDisplay, mWin);
		hidden = false;
	}
	else if (!show && !hidden)
	{
		::XDefineCursor(mDisplay, mWin, mInvisibleCursor);
		hidden = true;
	}
}

//==========================================================================================================
// Sets the graphics manager that will be associated with the window
//==========================================================================================================

bool SysWindow::SetGraphics (IGraphics* ptr)
{
	if (mWin == 0)
	{
		mGraphics = ptr;
	}
	else if (mGraphics == 0 && ptr != 0)
	{
		mGraphics = ptr;
		return _CreateContext();
	}
	else if (mGraphics != 0 && ptr == 0)
	{
		_ReleaseContext();
		mGraphics = 0;
	}
	return true;
}

//==========================================================================================================
// Sets the window's style
//==========================================================================================================

bool SysWindow::SetStyle (uint style)
{
	if (mStyle == style) return true;

	ASSERT(style != IWindow::Style::Child, "Child windows are not supported on X11!");

	mStyle = style;

	if (mWindowThread == Thread::GetID())
	{
		bool retVal (false);
		Lock();
		{
			mIgnoreResize = true;

			if (style == IWindow::Style::Fixed)
			{
				XSizeHints hints;
				hints.flags = PMinSize | PMaxSize;
				hints.min_width = hints.max_width = mSize.x;
				hints.min_height = hints.max_height = mSize.y;

				::XSetWMNormalHints(mDisplay, mWin, &hints);
			}
			else if (style == IWindow::Style::FullScreen)
			{
				XEvent xev;
				memset(&xev, 0, sizeof(xev));

				xev.type = ClientMessage;
				xev.xclient.window = mWin;
				xev.xclient.message_type = ::XInternAtom(mDisplay, "_NET_WM_STATE", false);
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = 1;
				xev.xclient.data.l[1] = ::XInternAtom(mDisplay, "_NET_WM_STATE_FULLSCREEN", false);
				xev.xclient.data.l[2] = 0;

				::XSendEvent(mDisplay, DefaultRootWindow(mDisplay), false, SubstructureNotifyMask, &xev);
			}

			mIgnoreResize = false;
		}
		Unlock();
		if (mHandler) mHandler->OnResize( GetSize() );
		return retVal;
	}
	DEBUG("SysWindow::SetStyle() called from a thread different than the one in which the window was created!");
	return false;
}

//=============================================================================================================

void SysWindow::BeginFrame()
{
	Lock();
	::glXMakeCurrent(mDisplay, mWin, mGLXContext);
}

//=============================================================================================================

void SysWindow::EndFrame()
{
	::glXSwapBuffers(mDisplay, mWin);
	Unlock();
}

//==========================================================================================================
// Retrieves a string from the clipboard
//==========================================================================================================

String SysWindow::GetClipboardText() const
{
	// TODO: Currently this function does not request the TARGETS atom, to see if XA_STRING is a supported target
	// instead it just assumes XA_STRING is supported. 
	// As soon as the engine supports UTF8 strings, this should probably be changed

	Atom clipboard = XInternAtom(mDisplay, "CLIPBOARD", True);
	Window owner = XGetSelectionOwner(mDisplay, clipboard);

	if (owner != None)
	{
		if (owner == mWin) return mClipboard;

		// Request the selection as XA_STRING
		XConvertSelection(mDisplay, clipboard, XA_STRING, clipboard, mWin, CurrentTime);
		XFlush (mDisplay);
		XEvent e;

		// Wait until the selection owner sends us a SelectionNotify event, confirming it has sent us the selection
		while (!XCheckTypedWindowEvent(mDisplay, mWin, SelectionNotify, &e));

		if (e.xselection.property != None)
		{
			Atom type;
			int format;
			unsigned long dummy, bytes, length;
			unsigned char *data;

			XGetWindowProperty(	mDisplay, mWin, clipboard, 0, 0, False, AnyPropertyType,
				&type, &format, &length, &bytes, &data);

			if (bytes)
			{
				XGetWindowProperty(mDisplay, mWin, clipboard, 0, bytes, False,
					AnyPropertyType, &type, &format, &length, &dummy, &data);
			}
			return String((const char*)data);
		}
	}
	return String();
}

//==========================================================================================================
// Sets the system clipboard text
//==========================================================================================================

void SysWindow::SetClipboardText (const String& text)
{
	Atom clipboard = XInternAtom(mDisplay, "CLIPBOARD", true);
	XSetSelectionOwner(mDisplay, clipboard, mWin, CurrentTime);
	mClipboard = text;
}

#endif
