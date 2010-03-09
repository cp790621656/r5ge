#include "../Include/_All.h"

#ifdef _MACOS

#import <Cocoa/Cocoa.h>
#import "_CocoaClasses.h"
using namespace R5;

#define TOP_LEFT_ORIGIN

//============================================================================================================
// Cocoa managed memory pool
//============================================================================================================

unsigned int g_count = 0;

ManagedMemory::ManagedMemory() : mPool(nil)
{
	Lock();
	{
		if (++g_count == 1)
		{
			mPool = [[NSAutoreleasePool alloc] init];
			[NSApplication sharedApplication];
		}
	}
	Unlock();
}

//============================================================================================================
	
ManagedMemory::~ManagedMemory()
{
	Lock();
	{
		if (--g_count == 0)
		{
			[mPool drain];
			mPool = nil;
		}
	}
	Unlock();
}

//============================================================================================================
// Mac OS keycodes need to be converted to DirectInput-based keycodes (unsigned char limit)
//============================================================================================================

const unsigned char keymap[] =
{
	Key::A,
	Key::S,
	Key::D,
	Key::F,
	Key::H,
	Key::G,
	Key::Z,
	Key::X,
	Key::C,
	Key::V,
	Key::Unknown,
	Key::B,
	Key::Q,
	Key::W,
	Key::E,
	Key::R,
	Key::Y,
	Key::T,
	Key::One,
	Key::Two,
	Key::Three,
	Key::Four,
	Key::Six,
	Key::Five,
	Key::Equals,
	Key::Nine,
	Key::Seven,
	Key::Minus,
	Key::Eight,
	Key::Zero,
	Key::RightBracket,
	Key::O,
	Key::U,
	Key::LeftBracket,
	Key::I,
	Key::P,
	Key::Return,
	Key::L,
	Key::J,
	Key::Apostrophe,
	Key::K,
	Key::Semicolon,
	Key::Slash,
	Key::Comma,
	Key::Backslash,
	Key::N,
	Key::M,
	Key::Period,
	Key::Tab,
	Key::Space,
	Key::Grave,
	Key::Backspace,
	Key::Unknown,
	Key::Escape,
	Key::RightWindows,
	Key::LeftWindows,
	Key::LeftShift,
	Key::CapsLock,
	Key::LeftAlt,
	Key::LeftControl,
	Key::RightShift,
	Key::RightAlt,
	Key::RightControl,
	Key::Unknown,
	Key::Unknown,
	Key::Decimal,
	Key::Unknown,
	Key::Multiply,
	Key::Unknown,
	Key::Add,
	Key::Unknown,
	Key::NumLock,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::Divide,
	Key::NumpadEnter,
	Key::Unknown,
	Key::Subtract,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::Numpad0,
	Key::Numpad1,
	Key::Numpad2,
	Key::Numpad3,
	Key::Numpad4,
	Key::Numpad5,
	Key::Numpad6,
	Key::Numpad7,
	Key::Unknown,
	Key::Numpad8,
	Key::Numpad9,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::F5,
	Key::F6,
	Key::F7,
	Key::F3,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::PrintScreen,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::AppMenu,
	Key::Unknown,
	Key::Unknown,
	Key::Unknown,
	Key::Insert,
	Key::Home,
	Key::PageUp,
	Key::Delete,
	Key::F4,
	Key::End,
	Key::F2,
	Key::PageDown,
	Key::F1,
	Key::ArrowLeft,
	Key::ArrowRight,
	Key::ArrowDown,
	Key::ArrowUp
};

//============================================================================================================
// Inline function to convert keycodes from mac to directinput-based codes
//============================================================================================================

inline unsigned char GetKeyCode (unsigned short key)
{
	return (key < sizeof(keymap)) ? keymap[key] : Key::Unknown;
}

//============================================================================================================
// Handle the event, whatever it may be
//============================================================================================================

bool HandleEvent (NSEvent* event)
{
	// Ignore null windows
	NSWindow* win = [event window];

	if (win == nil)
	{
		[NSApp sendEvent:event];
		return true;
	}

	// Ignore windows with no views
	MyView* view = [win contentView];
	if (view == nil)
	{
		[NSApp sendEvent:event];
		return true;
	}

	// Get the associated event handler and mouse position
	IEventReceiver*  handler = [view GetHandler];
	Vector2i&		mouse	= [view GetMousePos];
	bool*			keys	= [view GetKeyStates];

	// Events have a type that determines which messages it responds to
	unsigned int eventType ( [event type] );

	if ( eventType  < NSMouseEntered	||
		 eventType == NSOtherMouseDown	||
		 eventType == NSOtherMouseUp	||
		 eventType == NSOtherMouseDragged )
	{
		unsigned int	button	( [event buttonNumber] );
		const NSPoint&	point	( [event locationInWindow] );
		const NSRect&	rect	( [view bounds] );

		// Convert the coordinates from screen space to window space
		[view convertPoint:point fromView:nil];

		// Check whether the coordinates are inside the view
		bool inside = [view mouse:point inRect:rect];

		// Update the mouse position
#ifdef TOP_LEFT_ORIGIN
		Vector2i current ( (short)Float::RoundToInt(point.x), (short)Float::RoundToInt(rect.size.height - point.y) );
#else
		Vector2i current ( (short)Float::RoundToInt(point.x), (short)Float::RoundToInt(point.y) );
#endif
		Vector2i delta ( current - mouse );
		mouse = current;

		if ( eventType == NSLeftMouseUp  ||
			 eventType == NSRightMouseUp ||
			 eventType == NSOtherMouseUp )
		{
			keys[Key::MouseLeft + button] = false;
			if (handler) handler->OnKeyPress(mouse, Key::MouseLeft + button, false);
		}
		else if (inside)
		{
			if ( eventType == NSMouseMoved		  ||
				 eventType == NSLeftMouseDragged  ||
				 eventType == NSRightMouseDragged ||
				 eventType == NSOtherMouseDragged )
			{
				if (handler != 0 && delta) handler->OnMouseMove(mouse, delta);
			}
			else
			{
				keys[Key::MouseLeft + button] = true;
				if (handler) handler->OnKeyPress(mouse, Key::MouseLeft + button, true);
			}
		}
	}
	else if ( eventType == NSScrollWheel )
	{
		if (handler) handler->OnScroll(mouse, [event deltaY]);
	}
	else if ( eventType == NSKeyDown )
	{
		unsigned char key ( GetKeyCode([event keyCode]) );
		keys[key] = true;

		// Whether the command key is current held down
		bool command = keys[Key::LeftWindows] || keys[Key::RightWindows];
		
		// Command+Q is a standard 'quit the app' signal for macs.
		// Is there another event it sends? I don't know... but checking keys here works fine.
		if (command && key == Key::Q) return false;

		if (handler)
		{
			handler->OnKeyPress(mouse, key, true);

			// Only process character events if the command key isn't held
			if (!command)
			{
				NSString* chars	( [event characters] );
				unsigned int length = [chars length];

				for (unsigned int i = 0; i < length; ++i)
				{
					unsigned short ch = [chars characterAtIndex:i];
					if (ch >= 32 && ch < 127) handler->OnChar((unsigned char)ch);
				}
			}

			// Don't forward this event to the app or it makes a beeping noise
			return true;
		}
	}
	else if ( eventType == NSKeyUp )
	{
		unsigned char key ( GetKeyCode([event keyCode]) );
		keys[key] = false;

		if (handler) handler->OnKeyPress(mouse, key, false);
	}
	else if ( eventType == NSFlagsChanged )	// Modifier keys (Shift, Control, etc)
	{
		unsigned short nsKey ( [event keyCode] );
		unsigned int flags ( [event modifierFlags] );
		unsigned char key ( GetKeyCode(nsKey) );

		// 32-bit flag, both sides must be unsigned
		unsigned int left  = (flags >> 16) & 0xFF;
		unsigned int right = flags & 0xFF;
		flags = ((left << 16) | right);

		// Determine whether the key is down
		bool isDown (false);

		// Flag masks here were determined by experimentation
		if		(key == Key::LeftShift)		isDown = flags & 0x020002;
		else if (key == Key::LeftControl)	isDown = flags & 0x040001;
		else if (key == Key::LeftWindows)	isDown = flags & 0x100008;
		else if (key == Key::LeftAlt)		isDown = flags & 0x080020;
		else if (key == Key::RightAlt)		isDown = flags & 0x080040;
		else if (key == Key::RightWindows)	isDown = flags & 0x100010;
		else if (key == Key::RightControl)	isDown = flags & 0x040000;
		else if (key == Key::RightShift)	isDown = flags & 0x020004;

		keys[key] = isDown;

		if (handler) handler->OnKeyPress(mouse, key, isDown);
	}

	// Forward this event to the rest of the Cocoa application
	[NSApp sendEvent:event];
	return true;
}

//============================================================================================================
// Process all queued messages
//============================================================================================================

bool SendCocoaEvents()
{
	NSEvent* event (nil);

	while (nil != (event = [NSApp
			nextEventMatchingMask:	NSAnyEventMask
			untilDate:				nil
			inMode:					NSDefaultRunLoopMode
			dequeue:				YES]))
	{
		if (!HandleEvent(event))
			return false;
	}
	return true;
}

//============================================================================================================
// Cocoa window class -- C++ Wrapper
//============================================================================================================

SysWindow::SysWindow () : mForceUpdate(false), mGraphics(0), mStyle(Style::Undefined), mPrevStyle(Style::Undefined)
{
	mView = [[MyView alloc] init];
	mWin = nil;
}

//============================================================================================================
	
SysWindow::~SysWindow()
{
	SetGraphics(0);
	mView = nil;
}

//============================================================================================================

void SysWindow::SetTitle (const String& title)
{
	mTitle = title;
	if (mWin != nil) [mWin setTitle: [NSString stringWithCString:title.GetBuffer()]];
}

//============================================================================================================
	
void SysWindow::SetEventHandler (IEventReceiver* handler)
{
	if (mView != nil) [mView SetHandler:handler];
}

//============================================================================================================

bool SysWindow::SetGraphics (IGraphics* ptr)
{
	if (mWin == nil)
	{
		mGraphics = ptr;
	}
	else if (mGraphics == 0 && ptr != 0)
	{
		mGraphics = ptr;
		 _CreateContext();
	}
	else if (mGraphics != 0 && ptr == 0)
	{
		_ReleaseContext();
		mGraphics = 0;
	}
	return true;
}

//============================================================================================================

void SysWindow::SetFocus()
{
	NSWindow* win = [mView window];

	if (win != nil)
	{
		[win makeKeyAndOrderFront:nil];
	}
}

//============================================================================================================
// Sets the window's position and size
//============================================================================================================

bool SysWindow::_Set (const Vector2i& pos, const Vector2i& size)
{
	if (mWin != nil)
	{
		if (mStyle == Style::FullScreen)
			_EnterFullScreen(false);

		NSRect rect;
		rect.origin.x = pos.x;
		rect.size.width = size.x;
		rect.size.height = size.y;

#ifdef TOP_LEFT_ORIGIN
		NSRect screen = [[NSScreen mainScreen] frame];
		rect.origin.y = screen.size.height - pos.y - size.y;
#else
		rect.origin.y = pos.y;
#endif
		
		// Adjust the rectangle to ensure that the content frame fits
		rect = [mWin frameRectForContentRect: rect];

		// Set the frame rectangle
		[mWin setFrame:rect display:YES animate:NO];
		return true;
	}
	return false;
}

//============================================================================================================
// Sets the window's position
//============================================================================================================

bool SysWindow::SetPosition (const Vector2i& pos)
{
	if (mStyle == Style::FullScreen || mPos == pos)
	{
		mPos = pos;
		return true;
	}
	return _Set(pos, mSize);
}

//============================================================================================================
// Sets the window's size
//============================================================================================================

bool SysWindow::SetSize (const Vector2i& size)
{
	if (mStyle == Style::FullScreen || mSize == size)
	{
		mSize = size;
		return true;
	}
	return _Set(mPos, size);
}

//============================================================================================================
// Changes the window's style
//============================================================================================================

bool SysWindow::SetStyle (unsigned int style)
{
	if (style == mStyle) return true;
	
	if (mWin != nil)
	{
		if (mStyle == Style::FullScreen)
			_EnterFullScreen(false);
		
		if (style == Style::FullScreen)
			_EnterFullScreen(true);
		
		mStyle = style;
		return true;
	}
	return false;
}

//============================================================================================================

bool SysWindow::IsMinimized() const
{
	return (mWin == nil) ? true : [mWin isMiniaturized];
}

//============================================================================================================

void SysWindow::ShowCursor(bool show)
{
	if (show)	CGDisplayShowCursor(kCGDirectMainDisplay);
	else		CGDisplayHideCursor(kCGDirectMainDisplay);
}

//============================================================================================================

void SysWindow::Close()
{
	if (mWin != nil)
	{
		[mWin setContentView:nil];
		[mWin close];
		mWin = nil;

		if (mStyle != Style::Undefined)
		{
			mPrevStyle = mStyle;
			mStyle = Style::Undefined;
		}
	}
}

//============================================================================================================

bool SysWindow::Create (const String&	title,
						short			x,
						short			y,
						unsigned short	w,
						unsigned short	h,
						unsigned int	style,
						unsigned short	iconID,
						unsigned short	cursorID,
						void*			pParent)
{
	if (mWin == nil && style != Style::Undefined)
	{
		ASSERT(pParent == 0, "Child windows are not yet supported");

#ifdef TOP_LEFT_ORIGIN
		NSRect screen = [[NSScreen mainScreen] frame];
		y = screen.size.height - y - h;
#endif

		mTitle = title;

		// Create the window
		mWin = [[MyWindow alloc]	initWithContentRect:	NSMakeRect(x, y, w, h)
									styleMask:				NSTitledWindowMask	  | 
															NSResizableWindowMask |
															NSClosableWindowMask  |
															NSMiniaturizableWindowMask
									backing:				NSBackingStoreBuffered
									defer:					NO];

		[mWin setContentView:mView];
		[mWin setTitle:[NSString stringWithCString:title.GetBuffer()]];
		[mWin makeKeyAndOrderFront:nil];
		[mWin setAcceptsMouseMovedEvents:YES];
		[mWin setReleasedWhenClosed:YES];

		mStyle = Style::Normal;

		if (style == Style::FullScreen)
			_EnterFullScreen(true);

		if (mGraphics == 0 || _CreateContext())
			return true;

		// Something failed
		Close();
	}
	return false;
}

//============================================================================================================
// Enter the full screen mode
//============================================================================================================

bool SysWindow::_EnterFullScreen (bool enter)
{
	if (mWin != nil)
	{
		if (enter)
		{
			NSScreen*		mainScreen	= [NSScreen mainScreen];
			NSDictionary*	screenInfo	= [mainScreen deviceDescription];
			NSNumber*		screenID	= [screenInfo objectForKey:@"NSScreenNumber"];
			const NSRect&	frame		= [mainScreen frame];

			CGDirectDisplayID displayID = (CGDirectDisplayID)[screenID longValue];

			if ( CGDisplayCapture(displayID) == kCGErrorSuccess )
			{
				NSDictionary* dict = [NSDictionary
					dictionaryWithObject:[NSNumber numberWithBool: NO]
					forKey:NSFullScreenModeAllScreens];

				if ([mView enterFullScreenMode:mainScreen withOptions:dict])
				{
					mFsSize.Set(frame.size.width, frame.size.height);
					mStyle = Style::FullScreen;
					mForceUpdate = true;
				}
			}
			else return false;
		}
		else
		{
			[mView exitFullScreenModeWithOptions:nil];
			mStyle = Style::Normal;
			mForceUpdate = true;
		}
	}
	return true;
}

//============================================================================================================
// Once-per-frame update function, returns whether the window is still valid
//============================================================================================================

bool SysWindow::Update ()
{
	if (!SendCocoaEvents())
	{
		Close();
		return false;
	}

	if (mWin != nil)
	{
		if ( [mWin wasClosed] )
		{
			mWin = nil;
		}
		else if (mView != nil)
		{
			// Minimized windows don't count
			if (IsMinimized()) return true;

			// There is no WM_SIZE equivalent, so the best way around it is to check the size of the view
			// every single frame. If it changes, send the Resize event informing the handler.
			const NSRect&	frame  ( [mWin frame] );
			const NSRect&	bounds ( [mView bounds] );

			Vector2i size (	(short)Float::RoundToInt(bounds.size.width),
							(short)Float::RoundToInt(bounds.size.height) );

			Vector2i pos (	(short)Float::RoundToInt(frame.origin.x),
							(short)Float::RoundToInt(frame.origin.y) );

#ifdef TOP_LEFT_ORIGIN
			NSRect screen = [[NSScreen mainScreen] frame];
			pos.y = screen.size.height - pos.y - size.y;
#endif

			if (mStyle != Style::FullScreen)
			{
				mPos = pos;

				if (mSize != size)
				{
					mSize = size;
					mForceUpdate = true;
				}
			}
			
			if (mForceUpdate)
			{
				mForceUpdate = false;

				// Update the context first
				_UpdateContext();

				// Inform the event handler
				IEventReceiver* handler = [mView GetHandler];
				if (handler) handler->OnResize( GetSize() );
			}
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Retrieves a string from the clipboard
//============================================================================================================

String SysWindow::GetClipboardText() const
{
	NSPasteboard*	pb		= [NSPasteboard generalPasteboard];
	NSString*		nsText	= [pb stringForType: NSStringPboardType];
	const char*		cString = [nsText UTF8String];

	String text; 
	cString >> text;
	return text;
}

//============================================================================================================
// Sets the system clipboard text
//============================================================================================================

void SysWindow::SetClipboardText (const String& text)
{
	NSPasteboard*	pb		= [NSPasteboard generalPasteboard];
	NSArray*		types	= [NSArray arrayWithObjects:NSStringPboardType, nil];
	NSString*		nsText	= [NSString stringWithCString:text.GetBuffer()];

	[pb declareTypes:types owner:nil];
	[pb setString:nsText forType:NSStringPboardType];
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool SysWindow::SerializeFrom (const TreeNode& root)
{
	bool retVal (true);

	// If the window has already been created, only support "Size" and "Fullscreen" tags
	if (mWin != nil)
	{
		Vector2i size(0, 0);
		unsigned int style = Style::Undefined;

		for (unsigned int i = 0; i < root.mChildren.GetSize(); ++i)
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
	else // If the window hasn't been created
	{
		String		title;
		Vector2i	size(1024, 768);
		Vector2i	pos(0, 0);
		bool		full (false);

		for (unsigned int i = 0; i < root.mChildren.GetSize(); ++i)
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
		retVal = title.IsValid() ? Create( title, pos.x, pos.y, size.x, size.y, full ? Style::FullScreen : Style::Normal ) : false;
	}
	return retVal;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool SysWindow::SerializeTo (TreeNode& root) const
{
	if (mTitle.IsEmpty()) return false;

	TreeNode& node = root.AddChild("Window");
	node.AddChild("Title", mTitle);
	node.AddChild("Position", mPos);
	node.AddChild("Size", mSize);

	unsigned int style = (mStyle == Style::Undefined) ? mPrevStyle : mStyle;

	node.AddChild("Full Screen", style == Style::FullScreen);
	return true;
}

#endif