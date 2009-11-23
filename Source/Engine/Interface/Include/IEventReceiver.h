#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Event handler -- what is called on events
//============================================================================================================

struct IEventReceiver
{
	virtual ~IEventReceiver() {}

	virtual bool OnChar		(byte key)										{ return false; }
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown)	{ return false; }
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ return false; }
	virtual bool OnScroll	(const Vector2i& pos, float delta)				{ return false; }
	virtual void OnResize	(const Vector2i& size) {}
};