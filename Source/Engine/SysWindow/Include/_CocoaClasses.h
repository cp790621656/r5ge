#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Interface for the custom view that holds an event handler along with the current mouse position
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _MACOS

@interface MyView : NSView
{
	R5::IEventReceiver*	mHandler;
	R5::Vector2i		mMousePos;
	bool				mKey[256];
}

- (id)					init;
- (R5::IEventReceiver*)	GetHandler;
- (R5::Vector2i&)		GetMousePos;
- (bool*)				GetKeyStates;

- (void) SetHandler: (R5::IEventReceiver*)handler;

@end

//============================================================================================================
// Interface for the custom window that will perform an action
//============================================================================================================

@interface MyWindow : NSWindow
{
	bool mWasClosed;
}

- (id) init;
- (bool) wasClosed;
- (void) close;

@end

#endif