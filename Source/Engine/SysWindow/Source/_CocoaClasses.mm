#include "../Include/_All.h"

#ifdef _MACOS

#import <Cocoa/Cocoa.h>
#import "_CocoaClasses.h"
using namespace R5;

//===================================================================================================================
// Custom View implementation
//===================================================================================================================

@implementation MyView

- (id)init
{
	memset(mKey, 0, 256);
	return [super init];
}

- (R5::IEventReceiver*)	GetHandler	 { return mHandler;  }
- (R5::Vector2i&)		GetMousePos  { return mMousePos; }
- (bool*)				GetKeyStates { return mKey;		 }

- (void) SetHandler: (R5::IEventReceiver*)handler { mHandler = handler; }

@end

//===================================================================================================================
// Custom window implementation
//===================================================================================================================

@implementation MyWindow

- (id) init
{
	mWasClosed = false;
	return [super init];
}

- (bool) wasClosed
{
	return mWasClosed;
}

- (void) close;
{
	mWasClosed = true;
	[super close];
}

@end

#endif