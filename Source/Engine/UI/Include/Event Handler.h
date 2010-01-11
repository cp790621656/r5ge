#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Event container class for triggered callbacks
//============================================================================================================

class UIRoot;
class UIArea;

class UIEventHandler
{
protected:
	typedef FastDelegate<bool (UIArea*, bool)>								OnMouseOverDelegate;
	typedef FastDelegate<bool (UIArea*, const Vector2i&, const Vector2i&)>	OnMouseMoveDelegate;
	typedef FastDelegate<bool (UIArea*, const Vector2i&, byte, bool)>		OnKeyDelegate;
	typedef FastDelegate<bool (UIArea*, const Vector2i&, float)>			OnScrollDelegate;
	typedef FastDelegate<bool (UIArea*, bool)>								OnFocusDelegate;
	typedef FastDelegate<bool (UIArea*)>									OnChangeDelegate;

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

	friend class UIRoot;

	void CopyEvents (const UIEventHandler& entry)
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