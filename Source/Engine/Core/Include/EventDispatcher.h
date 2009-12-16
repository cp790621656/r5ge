#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Event dispatcher has the ability to register event handling callbacks
//============================================================================================================

class EventDispatcher
{
public:

	typedef FastDelegate<void (void)>											OnDrawDelegate;
	typedef FastDelegate<bool (const Vector2i& pos, byte key, bool isDown)>		OnKeyDelegate;
	typedef FastDelegate<bool (const Vector2i& pos, const Vector2i& delta)>		OnMouseMoveDelegate;
	typedef FastDelegate<bool (const Vector2i& pos, float delta)>				OnScrollDelegate;
	typedef FastDelegate<bool (const TreeNode& root)>							SerializeFromDelegate;
	typedef FastDelegate<void (TreeNode& root)>									SerializeToDelegate;

protected:

	OnDrawDelegate			mOnDraw;
	OnKeyDelegate			mOnKey;
	OnMouseMoveDelegate		mOnMouseMove;
	OnScrollDelegate		mOnScroll;
	SerializeFromDelegate	mOnFrom;
	SerializeToDelegate		mOnTo;
	
	UpdateList	mPre;		// Called prior to scene update
	UpdateList	mPost;		// Called after the scene update
	UpdateList	mLate;		// Called after the GUI update

public:

	// Event listener registration
	void SetListener (const OnDrawDelegate&			callback)	{ mOnDraw		= callback; }
	void SetListener (const OnKeyDelegate&			callback)	{ mOnKey		= callback; }
	void SetListener (const OnMouseMoveDelegate&	callback)	{ mOnMouseMove	= callback; }
	void SetListener (const OnScrollDelegate&		callback)	{ mOnScroll		= callback; }
	void SetListener (const SerializeFromDelegate&	callback)	{ mOnFrom		= callback; }
	void SetListener (const SerializeToDelegate&	callback)	{ mOnTo			= callback; }

	// Update callback registration
	void AddOnPreUpdate	 (const UpdateList::Callback& callback, float delay = 0.0f) { mPre.Add(callback, delay); }
	void AddOnPostUpdate (const UpdateList::Callback& callback, float delay = 0.0f) { mPost.Add(callback, delay); }
	void AddOnLateUpdate (const UpdateList::Callback& callback, float delay = 0.0f) { mLate.Add(callback, delay); }
};