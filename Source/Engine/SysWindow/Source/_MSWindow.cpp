#include "../Include/_All.h"
#include <windows.h>
using namespace R5;

//==========================================================================================================
// Converts Windows scan key code into a more descriptive DirectInput key code
//==========================================================================================================

inline byte GetKeyCode(uint key, uint state)
{
	return  ((state >> 16) & 0x0F00) ?
			(MapVirtualKey(key, 0) | 0x80) :
			((state >> 16) & 0x00FF);
}

//==========================================================================================================

inline Vector2i GetWindowPosition(HWND hWnd)
{
	RECT rect;
	::GetWindowRect(hWnd, &rect);
	return Vector2i( (short)rect.left, (short)rect.top );
}

//==========================================================================================================

inline ulong GetWindowsStyle(uint style)
{
	const ulong common		= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	const ulong titlebar	= WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
	const ulong resizeable	= WS_MAXIMIZEBOX | WS_THICKFRAME;

	if (style == IWindow::Style::Normal)		return common | titlebar | resizeable;
	if (style == IWindow::Style::Fixed)			return common | titlebar;
	if (style == IWindow::Style::Child)			return common | WS_VISIBLE | WS_CHILD;
	if (style == IWindow::Style::FullScreen)	return common | WS_VISIBLE | WS_POPUP | WS_EX_TOPMOST;

	return WS_POPUP;
}

//==========================================================================================================
// Most basic event processing function
//==========================================================================================================

long R5::GlobalEventHandler(HWND hWnd, UINT eventId, UINT first, UINT second)
{
	long retVal (0);
	SysWindow* win = (SysWindow*)(::GetWindowLongA(hWnd, GWLP_USERDATA));

	if ( win != 0 )
	{
		static Vector2i mousePos;
		R5::IEventReceiver* handler (win->mHandler);

		switch ( eventId )
		{
		case SysWindow::Event::MouseMove:
			{
				mousePos = second;
				static Vector2i last (mousePos);
				if (handler != 0) handler->OnMouseMove(mousePos, mousePos - last);
				last = mousePos;
			}
			break;
		
		case SysWindow::Event::KeyDown:
			if (handler != 0) handler->OnKeyPress(mousePos, GetKeyCode(first, second), true);
			break;
		
		case SysWindow::Event::KeyUp:
			if (handler != 0) handler->OnKeyPress(mousePos, GetKeyCode(first, second), false);
			break;

		case SysWindow::Event::LeftButtonDown:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseLeft, true);
			break;

		case SysWindow::Event::LeftButtonUp:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseLeft, false);
			break;

		case SysWindow::Event::RightButtonDown:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseRight, true);
			break;

		case SysWindow::Event::RightButtonUp:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseRight, false);
			break;

		case SysWindow::Event::MiddleButtonDown:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseMiddle, true);
			break;

		case SysWindow::Event::MiddleButtonUp:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseMiddle, false);
			break;

		case SysWindow::Event::XButtonDown:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseFour, true);
			break;

		case SysWindow::Event::XButtonUp:
			if (handler != 0) handler->OnKeyPress(mousePos, Key::MouseFour, false);
			break;

		case SysWindow::Event::MouseWheel:
			if (handler != 0) handler->OnScroll(mousePos, (float)GET_WHEEL_DELTA_WPARAM(first) / WHEEL_DELTA);
			break;

		case SysWindow::Event::Char:
			if (handler != 0 && first > 31 && first < 128) handler->OnChar(first);
			break;

		case SysWindow::Event::Resize:
			if ( !win->mIgnoreResize )
			{
				win->mIsMinimized = (first == SIZE_MINIMIZED);

				if ( !win->mIsMinimized && win->mStyle != IWindow::Style::FullScreen )
				{
					Vector2i size(second);
					win->mSize = size;
					if (handler != 0) handler->OnResize(size);
				}
			}
			break;

		case SysWindow::Event::Activate:
			if ( !win->mIgnoreResize )
			{
				if ( win->mStyle == IWindow::Style::FullScreen )
				{
					if ( LOWORD(first) == WA_INACTIVE )
					{
						::ShowWindow(win->mHWnd, SW_MINIMIZE);
					}
				}
			}
			break;

		case SysWindow::Event::Close:
			win->Close();
			return 0;

		case SysWindow::Event::Move:
			if (!win->mIgnoreResize && win->mStyle != IWindow::Style::FullScreen)
			{
				win->mPos = GetWindowPosition(hWnd);
			}
			break;
		}
	}
	return (long)::DefWindowProc(hWnd, eventId, first, second);
}

//==========================================================================================================
// Helper function -- adjusts the given rectangle, taking into account the provided window style
//==========================================================================================================

inline void AdjustWindowRectangle(RECT& rWindow, long width, long height, DWORD dwStyle)
{
	rWindow.left	= 0;
	rWindow.top		= 0;
	rWindow.right	= width;
	rWindow.bottom	= height;
	::AdjustWindowRect(&rWindow, dwStyle, 0);
}

//==========================================================================================================

void SysWindow::Post(UINT eventId, UINT first, UINT second)
{
	::PostMessage(mHWnd, eventId, first, second);
}

//==========================================================================================================

void SysWindow::ShowCursor(bool show)
{
	if (show)	{ while (::ShowCursor(1) <  0); }
	else		{ while (::ShowCursor(0) > -1); }
}

//==========================================================================================================
// Set up the default values for everything
//==========================================================================================================

SysWindow::SysWindow() :
	mHWnd			(0),
	mHParent		(0),
	mHDC			(0),
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
	mHInstance = ::GetModuleHandle(0);
}

//==========================================================================================================

SysWindow::~SysWindow()
{
	Close();

	if (mHInstance && mTitle.IsValid())
	{
#ifdef _DEBUG
		if (::UnregisterClass (mTitle.GetBuffer(), mHInstance) == 0) { ASSERT(false, "Failed to unregister the window class"); }
#else
		::UnregisterClass (mTitle.GetBuffer(), mHInstance);
#endif
	}
}

//==========================================================================================================
// Closes the window
//==========================================================================================================

void SysWindow::Close()
{
	if (mStyle != Style::Undefined)
	{
		mPrevStyle = mStyle;
		mStyle		= Style::Undefined;
		mHandler	= 0;

		ShowCursor(true);

		_ReleaseContext();
		_ReleaseHandles();
	}
}

//==========================================================================================================
// Registers the window class -- must only be called internally
//==========================================================================================================

void SysWindow::_Register( USHORT iconID, USHORT cursorID )
{
	WNDCLASS pWindow;
	memset(&pWindow, 0, sizeof(WNDCLASS));

	HICON icon = (iconID == 0) ? 0 : LoadIcon( mHInstance, MAKEINTRESOURCE(iconID) );

	HCURSOR cursor = (cursorID == 0) ? LoadCursor(0, IDC_ARROW) :
		LoadCursor(mHInstance, MAKEINTRESOURCE(cursorID));

	pWindow.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	pWindow.lpfnWndProc		= (WNDPROC)GlobalEventHandler;
	pWindow.hInstance		= mHInstance;
	pWindow.hIcon			= icon;
	pWindow.hCursor			= cursor;
	pWindow.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	pWindow.lpszClassName	= mTitle.GetBuffer();

	::RegisterClass(&pWindow);
}

//==========================================================================================================
// Creates the actual HWND handle
//==========================================================================================================

bool SysWindow::_CreateHandles()
{
	if (mHWnd == 0)
	{
		RECT rect;
		ulong winStyle = GetWindowsStyle( mStyle );

		if ( mStyle == IWindow::Style::Normal || mStyle == IWindow::Style::Fixed )
		{
			// Determine the dimensions of the window needed for this resolution
			AdjustWindowRectangle( rect, mSize.x, mSize.y, winStyle );

			// Create the window using calculated dimensions
			mHWnd = ::CreateWindow( mTitle.GetBuffer(), mTitle.GetBuffer(), winStyle, mPos.x, mPos.y,
				rect.right - rect.left, rect.bottom - rect.top, mHParent, 0, mHInstance, 0 );
		}
		else
		{
			mHWnd = ::CreateWindow( mTitle.GetBuffer(), mTitle.GetBuffer(), winStyle, 0, 0,
				mSize.x, mSize.y, mHParent, 0, mHInstance, 0 );
		}

		// Check the client area
		::GetClientRect(mHWnd, &rect);

		short width  = (short)(rect.right - rect.left);
		short height = (short)(rect.bottom - rect.top);

		// If the given client rectangle is smaller than requested, then the window must have shrunk
		if ( width < mSize.x || height < mSize.y )
			mSize.Set(width, height);

		ASSERT(mHWnd != 0, "Failed to create a window!");

		if (mHWnd)
		{
			// Get the device context
			mHDC = ::GetDC(mHWnd);
			ASSERT(mHDC != 0, "Failed to get the device context from the window!");

			// Store a pointer to this class
			::SetWindowLongA(mHWnd, GWLP_USERDATA, (LONG)this);
		}

		// Ensure that everything succeeded
		if (mHWnd != 0 && mHDC != 0)
		{
			mWindowThread = Thread::GetID();

#ifdef _DEBUG
			System::Log("[WINDOW]  Created a new window: '%s'", mTitle.GetBuffer());
			System::Log("          - Thread ID: %ld", mWindowThread);
#endif
			return true;
		}
		ASSERT(false, String("Could not create '%s'!", mTitle.GetBuffer()).GetBuffer());
		return false;
	}
	return true;
}

//==========================================================================================================
// Destroys the window and DC handles
//==========================================================================================================

void SysWindow::_ReleaseHandles()
{
	if (mHWnd)
	{
#ifdef _DEBUG
		ASSERT( mWindowThread == Thread::GetID(),
			"You must destroy the window in the same thread it which it was created" );
#endif

		::ChangeDisplaySettings(0, 0);
		::SetWindowLongA (mHWnd, GWLP_USERDATA, 0);

#ifdef _DEBUG
		ASSERT( ::ReleaseDC (mHWnd, mHDC) != 0, String("Failed to release the device context: %u",
			GetLastError()).GetBuffer() );
		ASSERT( ::DestroyWindow (mHWnd) != 0, String("Failed to destroy the window: %u",
			GetLastError()).GetBuffer() );
#else
		::ReleaseDC (mHWnd, mHDC);
		::DestroyWindow (mHWnd);
#endif

		mHWnd = 0;
		mHDC = 0;
		mWindowThread = 0;

#ifdef _DEBUG
		System::Log("[WINDOW]  Destroyed window '%s'", mTitle.GetBuffer());
#endif
	}
}

//==========================================================================================================
// Changes the display settings to the latest parameters
//==========================================================================================================

bool SysWindow::_Set (const Vector2i& pos, const Vector2i& size, uint style)
{
	ulong winStyle = GetWindowsStyle(style);

	::ShowWindow(mHWnd, SW_HIDE);
	{
		if (style == Style::FullScreen)
		{
			mFsSize.Set(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));

			::SetWindowLong(mHWnd, GWL_STYLE, winStyle);
			::SetWindowPos(mHWnd, HWND_TOP, 0, 0, mFsSize.x, mFsSize.y, SWP_FRAMECHANGED);
		}
		else
		{
			mPos = pos;
			mSize = size;

			// Determine the dimensions of the window with the regular window style
			RECT rect;
			AdjustWindowRectangle(rect, mSize.x, mSize.y, winStyle);

			::SetWindowLong(mHWnd, GWL_STYLE, winStyle);
			::SetWindowPos(mHWnd, HWND_TOP, mPos.x, mPos.y,
				rect.right - rect.left,
				rect.bottom - rect.top,
				SWP_NOACTIVATE);

			// Check the actual width and height
			::GetClientRect(mHWnd, &rect);

			short width  = (short)(rect.right - rect.left);
			short height = (short)(rect.bottom - rect.top);

			// If the width/height was shrunk, update own size and turn off the ignore resize messages flag
			if ( width < mSize.x || height < mSize.y )
			{
				mSize.Set(width, height);
				mIgnoreResize = false;
			}
		}

		mStyle = style;
	}
	::ShowWindow(mHWnd, SW_SHOW);
	return true;
}

//==========================================================================================================
// Creates a window of specified name, width, and height
//==========================================================================================================

bool SysWindow::Create(
	const String&	title,
    short			x,			// 0
	short			y,			// 0
	ushort			width,		// 1024
	ushort			height,		// 768
	uint			style,		// Style::Normal
	ushort			iconID,		// 0
	ushort			cursorID,	// 0
	void*			pParent)	// 0
{
	HWND hParent = (HWND)pParent;

	if ( mStyle != Style::Undefined || !title )		return false;
	if ( style == Style::Child && !IsWindow(hParent) )	return false;

	Lock();

	bool retVal		= true;
	mIgnoreResize	= true;
	mHParent		= hParent;
	mTitle			= title;
	mStyle			= style;

	mPos.Set(x, y);
	mSize.Set(width, height);

	// Register the window class
	_Register( iconID, cursorID );

	// Create the window
	if ( !_CreateHandles() )
	{
		mStyle = Style::Undefined;
		::UnregisterClass( mTitle.GetBuffer(), mHInstance );
		retVal = false;
	}

	if (retVal)
	{
		// Full screen mode needs some special processing
		if (mStyle == Style::FullScreen)
		{
			mStyle = Style::Undefined;
			_Set(mPos, mSize, Style::FullScreen);
		}

		if ( mGraphics != 0 && !_CreateContext() )
		{
			Thread::MessageWindow("Failed to initialize the graphics controller. Shutting down.");
			_ReleaseHandles();
			retVal = false;
		}
		else if (mHandler)
		{
			mHandler->OnResize(GetSize());
		}
	}

	if (retVal)
	{
		::ShowWindow	(mHWnd, SW_SHOWNORMAL);
		::UpdateWindow	(mHWnd);
		::SetFocus		(mHWnd);
	}

	mIgnoreResize = false;
	Unlock();
	return retVal;
}

//==========================================================================================================
// Changes the window's title
//==========================================================================================================

void SysWindow::SetTitle (const String& title)
{
	if (mHWnd != 0) ::SetWindowText(mHWnd, title.GetBuffer());
}

//==========================================================================================================
// Sets the graphics manager that will be associated with the window
//==========================================================================================================

bool SysWindow::SetGraphics (IGraphics* ptr)
{
	if (mHWnd == 0)
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
// Sets the window's position -- thread-safe
//==========================================================================================================

bool SysWindow::SetPosition (const Vector2i& pos)
{
	if (mStyle == Style::FullScreen || mPos == pos)
	{
		mPos = pos;
		return true;
	}

	if (mWindowThread == Thread::GetID())
	{
		bool retVal (false);
		Lock();
		{
			mIgnoreResize = true;
			retVal = _Set(pos, mSize, mStyle);
			mIgnoreResize = false;
		}
		Unlock();
		return retVal;
	}
	DEBUG("SysWindow::SetPosition() called from a thread different than the one in which the window was created!");
	return false;
}

//==========================================================================================================
// Sets the window's size -- thread-safe
//==========================================================================================================

bool SysWindow::SetSize (const Vector2i& size)
{
	if (mStyle == Style::FullScreen || mSize == size)
	{
		mSize = size;
		return true;
	}

	if (mWindowThread == Thread::GetID())
	{
		bool retVal (false);
		Lock();
		{
			mIgnoreResize = true;
			retVal = _Set(mPos, size, mStyle);
			mIgnoreResize = false;
		}
		Unlock();
		if (mHandler) mHandler->OnResize( GetSize() );
		return retVal;
	}
	DEBUG("SysWindow::SetSize() called from a thread different than the one in which the window was created!");
	return false;
}

//==========================================================================================================
// Changes the window's style
//==========================================================================================================

bool SysWindow::SetStyle (uint style)
{
	if (mStyle == style)
		return true;

	if (mWindowThread == Thread::GetID())
	{
		bool retVal (false);
		Lock();
		{
			mIgnoreResize = true;
			retVal = _Set(mPos, mSize, style);
			mIgnoreResize = false;
		}
		Unlock();
		if (mHandler) mHandler->OnResize( GetSize() );
		return retVal;
	}
	DEBUG("SysWindow::SetStyle() called from a thread different than the one in which the window was created!");
	return false;
}

//==========================================================================================================
// Gives focus to the window
//==========================================================================================================

void SysWindow::SetFocus()
{
	if (mWindowThread == Thread::GetID())
	{
		::SetFocus(mHWnd);
	}
}

//==========================================================================================================
// Processes all piled up messages and returns whether the application needs to exit
//==========================================================================================================

bool SysWindow::Update()
{
	if (mHWnd == 0) return false;

	if (mWindowThread == Thread::GetID())
	{
		MSG msg;

		while ( ::PeekMessage(&msg, mHWnd, 0, 0, PM_REMOVE) )	// Look at the message
		{
			::TranslateMessage(&msg);		// Translate virtual key messages (button keypresses, etc)
			::DispatchMessage(&msg);		// Send the message to the window
		}
		
		if (mIsMinimized) ::Sleep(5);
	}
	return mStyle != Style::Undefined;
}

//==========================================================================================================
// Retrieves a string from the clipboard
//==========================================================================================================

String SysWindow::GetClipboardText() const
{
	String text;
	::OpenClipboard(NULL);
	HANDLE clipboard = ::GetClipboardData(CF_TEXT);
	::CloseClipboard();
	((char*)::GlobalLock(clipboard)) >> text;
	::GlobalUnlock(clipboard);
	return text;
}

//==========================================================================================================
// Sets the system clipboard text
//==========================================================================================================

void SysWindow::SetClipboardText (const String& text)
{
	uint length = text.GetLength();
	HGLOBAL hText = ::GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, length + 1);
	char* pText = (char*)::GlobalLock(hText);
	memcpy(pText, text.GetBuffer(), length);
	pText[length] = 0;

	::GlobalUnlock(hText);
	::OpenClipboard(NULL);
	::EmptyClipboard();
	::SetClipboardData(CF_TEXT, hText);
	::CloseClipboard();
}

//==========================================================================================================
// Serialization -- loading
//==========================================================================================================

bool SysWindow::SerializeFrom (const TreeNode& root)
{
	bool retVal (false);

	// If the window has already been created, only support "Size" and "Fullscreen" tags
	if (mHWnd)
	{
		Vector2i size(0, 0);
		uint style = Style::Undefined;

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if ( tag == "Size" )
			{
				value >> size;
			}
			else if ( tag == "Full Screen" )
			{
				bool full;
				if (value >> full)
				{
					style = full ? Style::FullScreen : Style::Normal;
				}
			}
		}

		// Resize the window if necessary
		if (size != 0 || style != Style::Undefined)
		{
			retVal = SetSize(size) && SetStyle(style);
		}
	}
	// If the window hasn't been created, but there is an instance set, get all properties
	else if (mHInstance)
	{
		String		title;
		Vector2i	size(1024, 768);
		Vector2i	pos(0, 0);
		bool		full (false);

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if		( tag == "Name" || tag == "Title" )	value >> title;
			else if ( tag == "Size" )					value >> size;
			else if ( tag == "Position" )				value >> pos;
			else if ( tag == "Full Screen" )			value >> full;
		}

		// If there's at the very least a name present, create a window
		retVal = (title.IsValid() ? Create( title, pos.x, pos.y, size.x, size.y, full ? Style::FullScreen : Style::Normal ) : false);
	}
	return retVal;
}

//==========================================================================================================
// Serialization -- Save
//==========================================================================================================

bool SysWindow::SerializeTo (TreeNode& root) const
{
	if (mTitle.IsEmpty()) return false;

	TreeNode& node = root.AddChild("Window");
	node.AddChild("Title", mTitle);
	node.AddChild("Position", mPos);
	node.AddChild("Size", mSize);
	node.AddChild("Full Screen", GetStyle() == Style::FullScreen);
	return true;
}