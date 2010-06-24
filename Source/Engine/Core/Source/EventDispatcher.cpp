#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Macroed as the code below repeats the same functionality
//============================================================================================================

#define ADD_FUNCTION(val) \
	void EventDispatcher::AddOn##val (const On##val##Delegate& callback, uint priority, bool threadSafe) \
	{													\
		if (threadSafe) mOn##val.Lock();				\
		{												\
			mOn##val.Expand().Set(priority, callback);	\
			mOn##val.Sort();							\
		}												\
		if (threadSafe) mOn##val.Unlock();				\
	}													\
														\
	void EventDispatcher::RemoveOn##val (const On##val##Delegate& callback, uint priority, bool threadSafe) \
	{										\
		if (threadSafe) mOn##val.Lock();	\
		mOn##val.Remove(Pair<On##val##Delegate>(priority, callback)); \
		if (threadSafe) mOn##val.Unlock();	\
	}

//============================================================================================================
// Add/remove functions using the macro above
//============================================================================================================

ADD_FUNCTION(Draw);
ADD_FUNCTION(Key);
ADD_FUNCTION(MouseMove);
ADD_FUNCTION(Scroll);

//============================================================================================================
// Trigger all draw callbacks
//============================================================================================================

bool EventDispatcher::HandleOnDraw()
{
	if (mOnDraw.IsValid())
	{
		mOnDraw.Lock();
		{
			for (uint i = mOnDraw.GetSize(); i > 0; )
			{
				mOnDraw[--i].callback();
			}
		}
		mOnDraw.Unlock();
		return true;
	}
	return false;
}

//============================================================================================================
// Handle the OnKey functionality
//============================================================================================================

bool EventDispatcher::HandleOnKey (const Vector2i& pos, byte key, bool isDown)
{
	if (mOnKey.IsValid())
	{
		mOnKey.Lock();
		{
			// Run through the registered callbacks in reverse order
			for (uint i = mOnKey.GetSize(); i > 0; )
			{
				// Trigger the callback
				uint retVal = mOnKey[--i].callback(pos, key, isDown);

				// If the function wants to be removed, do that now
				if ((retVal & EventDispatcher::EventResponse::Remove) != 0) mOnKey.RemoveAt(i);

				// If the event was marked as handled, we're done
				if ((retVal & EventDispatcher::EventResponse::Handled) != 0)
				{
					mOnKey.Unlock();
					return true;
				}
			}
		}
		mOnKey.Unlock();
	}
	return false;
}

//============================================================================================================
// Handle mouse movement
//============================================================================================================

bool EventDispatcher::HandleOnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mOnMouseMove.IsValid())
	{
		mOnMouseMove.Lock();
		{
			// Run through the registered callbacks in reverse order
			for (uint i = mOnMouseMove.GetSize(); i > 0; )
			{
				// Trigger the callback
				uint retVal = mOnMouseMove[--i].callback(pos, delta);

				// If the function wants to be removed, do that now
				if ((retVal & EventDispatcher::EventResponse::Remove) != 0) mOnMouseMove.RemoveAt(i);

				// If the event was marked as handled, we're done
				if ((retVal & EventDispatcher::EventResponse::Handled) != 0)
				{
					mOnMouseMove.Unlock();
					return true;
				}
			}
		}
		mOnMouseMove.Unlock();
	}
	return false;
}

//============================================================================================================
// Handle mouse or touch pad scrolling
//============================================================================================================

bool EventDispatcher::HandleOnScroll (const Vector2i& pos, float delta)
{
	if (mOnScroll.IsValid())
	{
		mOnScroll.Lock();
		{
			// Run through the registered callbacks in reverse order
			for (uint i = mOnScroll.GetSize(); i > 0; )
			{
				// Trigger the callback
				uint retVal = mOnScroll[--i].callback(pos, delta);

				// If the function wants to be removed, do that now
				if ((retVal & EventDispatcher::EventResponse::Remove) != 0) mOnScroll.RemoveAt(i);

				// If the event was marked as handled, we're done
				if ((retVal & EventDispatcher::EventResponse::Handled) != 0)
				{
					mOnScroll.Unlock();
					return true;
				}
			}
		}
		mOnScroll.Unlock();
	}
	return false;
}