#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Event dispatcher has the ability to register event handling callbacks and is used by the Core class.
// Note that for the R5 user interface it's a better idea to use the built-in event functionality.
// These functions are here for object scripts, allowing them to register and unregister listeners.
//============================================================================================================

class EventDispatcher
{
public:

	struct EventResponse
	{
		enum
		{
			NotHandled	= 0,
			Handled		= 1,
			Remove		= 2,
		};
	};

	typedef FastDelegate<void (void)>											OnDrawDelegate;
	typedef FastDelegate<uint (const Vector2i& pos, byte key, bool isDown)>		OnKeyDelegate;
	typedef FastDelegate<uint (const Vector2i& pos, const Vector2i& delta)>		OnMouseMoveDelegate;
	typedef FastDelegate<uint (const Vector2i& pos, float delta)>				OnScrollDelegate;

private:

	template <typename Type>
	struct Pair
	{
		uint priority;
		Type callback;

		Pair() {}
		Pair(uint pri, const Type& val) : priority(pri), callback(val) {}

		bool operator < (const Pair<Type>& val) const { return priority < val.priority; }
		bool operator == (const Type& val) const { return callback == val; }
		bool operator == (uint idx) const { return priority == idx; }
		bool operator == (const Pair<Type>& val) { return (callback == val.callback) && (priority == val.priority); }
		void Set (uint idx, const Type& val) { priority = idx; callback = val; }
	};

	Array< Pair<OnDrawDelegate> >		mOnDraw;
	Array< Pair<OnKeyDelegate> >		mOnKey;
	Array< Pair<OnMouseMoveDelegate> >	mOnMouseMove;
	Array< Pair<OnScrollDelegate> >		mOnScroll;

protected:

	UpdateList	mPreList;	// Called prior to scene update
	UpdateList	mPostList;	// Called after the scene update
	UpdateList	mLateList;	// Called after the GUI update

public:

	// Event listener registration -- higher priority is handled first
	void AddOnDraw			(const OnDrawDelegate& callback, uint priority = 1000);
	void RemoveOnDraw		(const OnDrawDelegate& callback, uint priority);
	void RemoveOnDraw		(const OnDrawDelegate& callback);
	void AddOnKey			(const OnKeyDelegate& callback, uint priority = 1000);
	void RemoveOnKey		(const OnKeyDelegate& callback, uint priority);
	void RemoveOnKey		(const OnKeyDelegate& callback);
	void AddOnMouseMove		(const OnMouseMoveDelegate& callback, uint priority = 1000);
	void RemoveOnMouseMove	(const OnMouseMoveDelegate& callback, uint priority);
	void RemoveOnMouseMove	(const OnMouseMoveDelegate& callback);
	void AddOnScroll		(const OnScrollDelegate& callback, uint priority = 1000);
	void RemoveOnScroll		(const OnScrollDelegate& callback, uint priority);
	void RemoveOnScroll		(const OnScrollDelegate& callback);

	// Update callback registration -- the execution delay is in seconds
	void AddOnPreUpdate		(const UpdateList::Callback& callback, float delay = 0.0f)	{ mPreList.Add(callback, delay); }
	void RemoveOnPreUpdate	(const UpdateList::Callback& callback)						{ mPreList.Remove(callback); }
	void AddOnPostUpdate	(const UpdateList::Callback& callback, float delay = 0.0f)	{ mPostList.Add(callback, delay); }
	void RemoveOnPostUpdate	(const UpdateList::Callback& callback)						{ mPostList.Remove(callback); }
	void AddOnLateUpdate	(const UpdateList::Callback& callback, float delay = 0.0f)	{ mLateList.Add(callback, delay); }
	void RemoveOnLateUpdate	(const UpdateList::Callback& callback)						{ mLateList.Remove(callback); }

protected:

	// Functions called by the Core class that forward the events to registered listener callbacks
	bool HandleOnDraw();
	bool HandleOnKey		(const Vector2i& pos, byte key, bool isDown);
	bool HandleOnMouseMove	(const Vector2i& pos, const Vector2i& delta);
	bool HandleOnScroll		(const Vector2i& pos, float delta);
};