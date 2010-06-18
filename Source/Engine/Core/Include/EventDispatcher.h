#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Event dispatcher has the ability to register event handling callbacks and is used by the Core class.
// Note that this functionality pre-dates object and widget scripts, and should only be used in select cases.
// It's usually a more elegant solution to add a custom script to an object somewhere in the scene.
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
	
	UpdateList	mPreList;	// Called prior to scene update
	UpdateList	mPostList;	// Called after the scene update
	UpdateList	mLateList;	// Called after the GUI update
	UpdateList	mDrawList;	// Called during the draw process

public:

	// Event listener registration
	void SetListener (const OnDrawDelegate&			callback)	{ mOnDraw		= callback; } // Deprecated, use AddOnDraw
	void SetListener (const OnKeyDelegate&			callback)	{ mOnKey		= callback; }
	void SetListener (const OnMouseMoveDelegate&	callback)	{ mOnMouseMove	= callback; }
	void SetListener (const OnScrollDelegate&		callback)	{ mOnScroll		= callback; }
	void SetListener (const SerializeFromDelegate&	callback)	{ mOnFrom		= callback; } // Deprecated, use scripts
	void SetListener (const SerializeToDelegate&	callback)	{ mOnTo			= callback; } // Deprecated, use scripts

	// Update callback registration
	void AddOnPreUpdate		(const UpdateList::Callback& callback, float delay = 0.0f)	{ mPreList.Add(callback, delay); }
	void RemoveOnPreUpdate	(const UpdateList::Callback& callback)						{ mPreList.Remove(callback); }
	void AddOnPostUpdate	(const UpdateList::Callback& callback, float delay = 0.0f)	{ mPostList.Add(callback, delay); }
	void RemoveOnPostUpdate	(const UpdateList::Callback& callback)						{ mPostList.Remove(callback); }
	void AddOnLateUpdate	(const UpdateList::Callback& callback, float delay = 0.0f)	{ mLateList.Add(callback, delay); }
	void RemoveOnLateUpdate	(const UpdateList::Callback& callback)						{ mLateList.Remove(callback); }
	void AddOnDraw			(const UpdateList::Callback& callback, float delay = 0.0f)	{ mDrawList.Add(callback, delay); }
	void RemoveOnDraw		(const UpdateList::Callback& callback)						{ mDrawList.Remove(callback); }
};