#include "../Include/_All.h"

#ifdef _LINUX

#include <unistd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

//#define GLX_GLXEXT_PROTOTYPES
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
	return key - 8;
}

//============================================================================================================
// Set up the default values for everything
//============================================================================================================

SysWindow::SysWindow(uint MSAA) :
	mWindowThread	(0),
	mWin			(0),
	mStyle			(Style::Undefined),
	mPrevStyle		(Style::Undefined),
	mIsMinimized	(false),
	mIgnoreResize	(false),
	mPos			(0),
	mSize			(0),
	mHandler		(0),
	mGraphics		(0),
	mMSAA			(MSAA)
{
	mDisplay = XOpenDisplay(0);
}

//============================================================================================================

SysWindow::~SysWindow()
{
	if (mWin != None) Close();

	_ReleaseContext();

	::XDestroyWindow(mDisplay, mWin);

	mInvisibleCursor = 0;
	mWin = 0;

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
	if (mGraphics != 0) mGraphics->Release();
	if (mDisplay) ::glXMakeCurrent(mDisplay, None, NULL);
}

bool _GetCompatibleVisual(Display *display, XVisualInfo *vi, GLXFBConfig *fbConfig, uint *msaa)
{
	ASSERT(display != NULL, "display must not be NULL");

	int screen;
	screen = ::XDefaultScreen(display);

	int depth;
	depth = DefaultDepth(display, screen);

	int numElements;

	GLXFBConfig *fbConfigs = ::glXGetFBConfigs(display, screen, &numElements);

	if (fbConfigs)
	{
		// Correct msaa if it's not a power of 2
		uint bits = 0;
		do 
		{
			bits += 1;
			*msaa = (*msaa>>1);
		}
		while (*msaa > 1);

		*msaa = (*msaa<<bits);

		// Look for a GLXFBConfig with the needed attributes that has as many samples as possible 
		// and correct msaa accordingly
		int bestVal = -1;
		XVisualInfo *tmpVi;
		XVisualInfo *bestVi = NULL;
		int n = -1;
		for (int i = 0; i < numElements; i++)
		{
			int val;
			
			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_X_RENDERABLE, &val);
			if (val != True) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_DRAWABLE_TYPE, &val);
			if ( !(val & GLX_WINDOW_BIT) ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_RENDER_TYPE, &val);
			if ( !(val & GLX_RGBA_BIT) ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_X_VISUAL_TYPE, &val);
			if ( val != GLX_TRUE_COLOR ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_RED_SIZE, &val);
			if ( val != 8 ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_GREEN_SIZE, &val);
			if ( val != 8 ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_BLUE_SIZE, &val);
			if ( val != 8 ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_ALPHA_SIZE, &val);
			if ( val != 8 ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_DEPTH_SIZE, &val);
			if ( val != 24 ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_STENCIL_SIZE, &val);
			if ( val != 8 ) continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_DOUBLEBUFFER, &val);
			if ( val != True ) continue;


			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_SAMPLES, &val);
			if ( val > bestVal && val <= (int)*msaa ) 
			{
				tmpVi = ::glXGetVisualFromFBConfig(display, fbConfigs[i]);

				if ( tmpVi->depth != depth )
				{
					XFree(tmpVi);
					continue;
				}
				else
				{
					if ( bestVi != NULL ) XFree(bestVi);

					bestVi = tmpVi;
					bestVal = val;
					n = i;

					if ( val == (int)*msaa ) break;
				}
			}
		}

		if (n == -1) return false;

		*vi = *bestVi;
		*fbConfig = fbConfigs[n];
		*msaa = (uint)bestVal;

		XFree(bestVi);
		XFree(fbConfigs);

		return true;
	}

	return false;
}

//============================================================================================================
// Creates a window of specified name, width, and height
//============================================================================================================

bool SysWindow::Create(
	const	String& title,
	short	x,
	short	y,
	ushort	width,
	ushort	height,
	uint	style)
{
	Lock();

	bool retVal = false;

	if ( mDisplay )
	{

		if ( mWin == None )
		{
			XVisualInfo vi;
			GLXFBConfig fbConfig;

			if ( _GetCompatibleVisual(mDisplay, &vi, &fbConfig, &mMSAA) )
			{
				int screen;
				screen = ::XDefaultScreen(mDisplay);

				Window root;
				root = ::XDefaultRootWindow(mDisplay);

#ifndef NO_OPENGL3 // in case it's needed in the future to target non-OpenGL3 platforms
#ifndef GL_GLX_PROTOTYPES
				PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = 
					(PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress((const GLubyte*) "glXCreateContextAttribsARB");
				if (glXCreateContextAttribsARB == NULL)
				{
					mGLXContext = ::glXCreateContext(mDisplay, &vi, NULL, GL_TRUE);
				}
				else
#endif
				{
					const int attribList[] = {
						GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
						GLX_CONTEXT_MINOR_VERSION_ARB, 0,
						None
					};

					mGLXContext = glXCreateContextAttribsARB(mDisplay, fbConfig, 0, True, attribList);
				}
#else
				mGLXContext = ::glXCreateContext(mDisplay, &vi, NULL, GL_TRUE);
#endif

				XSetWindowAttributes swa;
				swa.colormap			= ::XCreateColormap(mDisplay, root, vi.visual, AllocNone);
				swa.event_mask		 	= PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
											KeyPressMask | KeyReleaseMask | StructureNotifyMask;
				swa.background_pixel	= BlackPixel(mDisplay, screen);

				mWin = ::XCreateWindow(	mDisplay, root, x, y, width, height,
										0, vi.depth, InputOutput, vi.visual,
										CWColormap | CWEventMask | CWBackPixel, &swa );

				if ( mWin != None )
				{
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

					retVal = _CreateContext();
				}

			}
		}
	
	}
	Unlock();	
	return retVal;
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
					KeySym xk;
					// = XLookupKeysym(&xev.xkey, 0);
					char buffer[4];
					XLookupString(&xev.xkey, buffer, sizeof(buffer), &xk, NULL);

					if (xk < 128)
					{
						mHandler->OnChar((char)xk);
					}

					if (!KeyRepeat)
					{
						mHandler->OnKeyPress(Vector2i(xev.xkey.x, xev.xkey.y), ConvertKeyCode(xev.xkey.keycode), true);
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
					mHandler->OnKeyPress(Vector2i(xev.xkey.x, xev.xkey.y), ConvertKeyCode(xev.xkey.keycode), false);
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
					Vector2i mousePos(xev.xmotion.x, xev.xmotion.y);
					mHandler->OnMouseMove(mousePos, mousePos - prevPos);
					prevPos = mousePos;
				}
				break;

				case ClientMessage:
					if (xev.xclient.data.l[0] == (long)wmDeleteMessage)
					{
						Close();
						return mStyle != Style::Undefined;
					}
					break;

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
				break;
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
