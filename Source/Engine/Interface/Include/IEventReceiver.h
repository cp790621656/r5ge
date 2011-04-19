#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Event handler -- what is called on events
// Author: Michael Lyashenko
//============================================================================================================

struct IEventReceiver
{
	virtual ~IEventReceiver() {}

	virtual bool OnChar		(byte key)										{ return false; }
	virtual bool OnKeyPress	(const Vector2i& pos, byte key, bool isDown)	{ return false; }
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ return false; }
	virtual bool OnScroll	(const Vector2i& pos, float delta)				{ return false; }
	virtual void OnResize	(const Vector2i& size) {}
};