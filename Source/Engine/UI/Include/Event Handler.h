#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Event container class for triggered callbacks
//============================================================================================================

class Root;
class Area;

class EventHandler
{
protected:
	typedef FastDelegate<bool (Area*, bool)>								OnMouseOverDelegate;
	typedef FastDelegate<bool (Area*, const Vector2i&, const Vector2i&)>	OnMouseMoveDelegate;
	typedef FastDelegate<bool (Area*, const Vector2i&, byte, bool)>			OnKeyDelegate;
	typedef FastDelegate<bool (Area*, const Vector2i&, float)>				OnScrollDelegate;
	typedef FastDelegate<bool (Area*, bool)>								OnFocusDelegate;
	typedef FastDelegate<bool (Area*)>										OnChangeDelegate;

	OnMouseOverDelegate		mOnMouseOver;
	OnMouseMoveDelegate		mOnMouseMove;
	OnKeyDelegate			mOnKey;
	OnScrollDelegate		mOnScroll;
	OnFocusDelegate			mOnSelect;
	OnChangeDelegate		mOnStateChange;
	OnChangeDelegate		mOnValueChange;

public:

	void SetOnMouseOver		(const OnMouseOverDelegate& fnct)	{ mOnMouseOver		= fnct; }
	void SetOnMouseMove		(const OnMouseMoveDelegate& fnct)	{ mOnMouseMove		= fnct; }
	void SetOnKey			(const OnKeyDelegate&		fnct)	{ mOnKey			= fnct; }
	void SetOnScroll		(const OnScrollDelegate&	fnct)	{ mOnScroll			= fnct; }
	void SetOnFocus			(const OnFocusDelegate&		fnct)	{ mOnSelect			= fnct; }
	void SetOnStateChange	(const OnChangeDelegate&	fnct)	{ mOnStateChange	= fnct; }
	void SetOnValueChange	(const OnChangeDelegate&	fnct)	{ mOnValueChange	= fnct; }

	const OnMouseOverDelegate&	GetOnMouseOver()	const { return mOnMouseOver;	}
	const OnMouseMoveDelegate&	GetOnMouseMove()	const { return mOnMouseMove;	}
	const OnKeyDelegate&		GetOnKey()			const { return mOnKey;			}
	const OnScrollDelegate&		GetOnScroll()		const { return mOnScroll;		}
	const OnFocusDelegate&		GetOnFocus()		const { return mOnSelect;		}
	const OnChangeDelegate&		GetOnStateChange()	const { return mOnStateChange;	}
	const OnChangeDelegate&		GetOnValueChange()	const { return mOnValueChange;	}

private:

	friend class Root;

	void CopyEvents (const EventHandler& entry)
	{
		if (entry.mOnMouseOver)		mOnMouseOver	= entry.mOnMouseOver;
		if (entry.mOnMouseMove)		mOnMouseMove	= entry.mOnMouseMove;
		if (entry.mOnKey)			mOnKey			= entry.mOnKey;
		if (entry.mOnScroll)		mOnScroll		= entry.mOnScroll;
		if (entry.mOnSelect)		mOnSelect		= entry.mOnSelect;
		if (entry.mOnStateChange)	mOnStateChange	= entry.mOnStateChange;
		if (entry.mOnValueChange)	mOnValueChange	= entry.mOnValueChange;
	}
};