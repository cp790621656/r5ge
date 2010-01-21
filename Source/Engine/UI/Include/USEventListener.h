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
	typedef FastDelegate<void (UIWidget*, bool)>								OnFocusDelegate;
	typedef FastDelegate<void (UIWidget*, uint, bool)>							OnStateDelegate;
	typedef FastDelegate<void (UIWidget*)>										OnValueDelegate;

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

	void SetOnMouseMove		(const OnMouseMoveDelegate& fnct)	{ mOnMouseMove		= fnct; }
	void SetOnKey			(const OnKeyDelegate&		fnct)	{ mOnKey			= fnct; }
	void SetOnScroll		(const OnScrollDelegate&	fnct)	{ mOnScroll			= fnct; }
	void SetOnMouseOver		(const OnMouseOverDelegate& fnct)	{ mOnMouseOver		= fnct; }
	void SetOnFocus			(const OnFocusDelegate&		fnct)	{ mOnFocus			= fnct; }
	void SetOnStateChange	(const OnStateDelegate&		fnct)	{ mOnStateChange	= fnct; }
	void SetOnValueChange	(const OnValueDelegate&		fnct)	{ mOnValueChange	= fnct; }

	const OnMouseMoveDelegate&	GetOnMouseMove()	const { return mOnMouseMove;	}
	const OnKeyDelegate&		GetOnKey()			const { return mOnKey;			}
	const OnScrollDelegate&		GetOnScroll()		const { return mOnScroll;		}
	const OnMouseOverDelegate&	GetOnMouseOver()	const { return mOnMouseOver;	}
	const OnFocusDelegate&		GetOnFocus()		const { return mOnFocus;		}
	const OnStateDelegate&		GetOnStateChange()	const { return mOnStateChange;	}
	const OnValueDelegate&		GetOnValueChange()	const { return mOnValueChange;	}

	// Virtual functions forward all events to bound delegates
	virtual bool OnMouseMove	(const Vector2i& pos, const Vector2i& delta)	{ return mOnMouseMove ? mOnMouseMove(mWidget, pos, delta) : false; }
	virtual bool OnKeyPress		(const Vector2i& pos, byte key, bool isDown)	{ return mOnKey ? mOnKey(mWidget, pos, key, isDown) : false; }
	virtual bool OnScroll		(const Vector2i& pos, float delta)				{ return mOnScroll ? mOnScroll(mWidget, pos, delta) : false; }
	virtual void OnMouseOver	(bool isMouseOver)								{ if (mOnMouseOver) mOnMouseOver(mWidget, isMouseOver); }
	virtual void OnFocus		(bool gotFocus)									{ if (mOnFocus) mOnFocus(mWidget, gotFocus); }
	virtual void OnStateChange	(uint state, bool isSet)						{ if (mOnStateChange) mOnStateChange(mWidget, state, isSet); }
	virtual void OnValueChange	()												{ if (mOnValueChange) mOnValueChange(mWidget); }
};