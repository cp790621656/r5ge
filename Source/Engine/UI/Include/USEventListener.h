#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Event listener script
//============================================================================================================

class USEventListener : public UIScript
{
public:

	typedef FastDelegate<bool (UIWidget*, bool)>								OnMouseOverDelegate;
	typedef FastDelegate<bool (UIWidget*, const Vector2i&, const Vector2i&)>	OnMouseMoveDelegate;
	typedef FastDelegate<bool (UIWidget*, const Vector2i&, byte, bool)>			OnKeyDelegate;
	typedef FastDelegate<bool (UIWidget*, const Vector2i&, float)>				OnScrollDelegate;
	typedef FastDelegate<bool (UIWidget*, bool)>								OnFocusDelegate;
	typedef FastDelegate<bool (UIWidget*, uint, bool)>							OnStateDelegate;
	typedef FastDelegate<bool (UIWidget*)>										OnValueDelegate;

private:

	OnMouseOverDelegate		mOnMouseOver;
	OnMouseMoveDelegate		mOnMouseMove;
	OnKeyDelegate			mOnKey;
	OnScrollDelegate		mOnScroll;
	OnFocusDelegate			mOnFocus;
	OnStateDelegate			mOnStateChange;
	OnValueDelegate			mOnValueChange;

public:

	R5_DECLARE_INHERITED_CLASS("USEventListener", USEventListener, UIScript, UIScript);

	void SetOnMouseOver		(const OnMouseOverDelegate& fnct)	{ mOnMouseOver		= fnct; }
	void SetOnMouseMove		(const OnMouseMoveDelegate& fnct)	{ mOnMouseMove		= fnct; }
	void SetOnKey			(const OnKeyDelegate&		fnct)	{ mOnKey			= fnct; }
	void SetOnScroll		(const OnScrollDelegate&	fnct)	{ mOnScroll			= fnct; }
	void SetOnFocus			(const OnFocusDelegate&		fnct)	{ mOnFocus			= fnct; }
	void SetOnStateChange	(const OnStateDelegate&		fnct)	{ mOnStateChange	= fnct; }
	void SetOnValueChange	(const OnValueDelegate&		fnct)	{ mOnValueChange	= fnct; }

	const OnMouseOverDelegate&	GetOnMouseOver()	const { return mOnMouseOver;	}
	const OnMouseMoveDelegate&	GetOnMouseMove()	const { return mOnMouseMove;	}
	const OnKeyDelegate&		GetOnKey()			const { return mOnKey;			}
	const OnScrollDelegate&		GetOnScroll()		const { return mOnScroll;		}
	const OnFocusDelegate&		GetOnFocus()		const { return mOnFocus;		}
	const OnStateDelegate&		GetOnStateChange()	const { return mOnStateChange;	}
	const OnValueDelegate&		GetOnValueChange()	const { return mOnValueChange;	}

	void CopyEvents (const USEventListener& entry)
	{
		if (entry.mOnMouseOver)		mOnMouseOver	= entry.mOnMouseOver;
		if (entry.mOnMouseMove)		mOnMouseMove	= entry.mOnMouseMove;
		if (entry.mOnKey)			mOnKey			= entry.mOnKey;
		if (entry.mOnScroll)		mOnScroll		= entry.mOnScroll;
		if (entry.mOnFocus)			mOnFocus		= entry.mOnFocus;
		if (entry.mOnStateChange)	mOnStateChange	= entry.mOnStateChange;
		if (entry.mOnValueChange)	mOnValueChange	= entry.mOnValueChange;
	}

	// Virtual functions forward all events to bound delegates
	virtual bool OnMouseOver	(bool isMouseOver)								{ return mOnMouseOver ? mOnMouseOver(mWidget, isMouseOver) : false; }
	virtual bool OnMouseMove	(const Vector2i& pos, const Vector2i& delta)	{ return mOnMouseMove ? mOnMouseMove(mWidget, pos, delta) : false; }
	virtual bool OnKeyPress		(const Vector2i& pos, byte key, bool isDown)	{ return mOnKey ? mOnKey(mWidget, pos, key, isDown) : false; }
	virtual bool OnScroll		(const Vector2i& pos, float delta)				{ return mOnScroll ? mOnScroll(mWidget, pos, delta) : false; }
	virtual bool OnFocus		(bool gotFocus)									{ return mOnFocus ? mOnFocus(mWidget, gotFocus) : false; }
	virtual bool OnStateChange	(uint state, bool isSet)						{ return mOnStateChange ? mOnStateChange(mWidget, state, isSet) : false; }
	virtual bool OnValueChange	()												{ return mOnValueChange ? mOnValueChange(mWidget) : false; }
};