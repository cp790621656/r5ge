#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple context menu
//============================================================================================================
// Note that the context menu class is not a standard UI widget. It's not serialized, and it's rebuilt only
// when it's shown. It's meant to be used for quick menus, not as a full-fledged UI widget.
//============================================================================================================

class UIContext : public UIAnimatedFrame
{
public:

	typedef Array<String> Entries;

protected:

	const UISkin*	mSkin;
	const IFont*	mFont;
	String			mFace;
	Color4ub		mTextColor;
	Color4ub		mBackColor;
	bool			mShadow;
	uint			mAlignment;
	Entries			mEntries;
	bool			mIsDirty;
	float			mMinWidth;
	Vector3f		mAnchor;
	Vector2f		mSize;
	String			mText;
	UISubPicture*	mHighlight;

public:

	UIContext();

	const UISkin*	GetSkin()		const	{ return mSkin; }
	const IFont*	GetFont()		const	{ return mFont; }
	const Color4ub&	GetTextColor()	const	{ return mTextColor; }
	const Color4ub&	GetBackColor()	const	{ return mBackColor; }
	bool			DropsShadow()	const	{ return mShadow;  }
	uint			GetAlignment()	const	{ return mAlignment; }
	const Entries&	GetAllEntries()	const	{ return mEntries; }
	float			GetMinWidth()	const	{ return mMinWidth; }
	const Vector3f&	GetAnchor()		const	{ return mAnchor; }
	const String&	GetText()		const	{ return mText; }

	void SetSkin		(const UISkin* skin)	{ mSkin		 = skin; mIsDirty = true; }
	void SetFace		(const String& face)	{ mFace		 = face; mIsDirty = true; }
	void SetFont		(const IFont* font)		{ mFont		 = font; mIsDirty = true; }
	void SetTextColor	(const Color4ub& c)		{ mTextColor = c;	 mIsDirty = true; }
	void SetBackColor	(const Color4ub& c)		{ mBackColor = c;	 mIsDirty = true; }
	void SetShadow		(bool val)				{ mShadow	 = val;	 mIsDirty = true; }
	void SetAlignment	(uint val)				{ mAlignment = val;	 mIsDirty = true; }
	void SetMinWidth	(float val)				{ mMinWidth	 = val;	 mIsDirty = true; }
	void SetAnchor		(const Vector3f& v)		{ mAnchor	 = v;	 mIsDirty = true; }

	// Clear all entries
	void ClearAllEntries();

	// Add a new entry to the selection
	void AddEntry (const String& entry);

protected:

	// Rebuild the list
	void _Rebuild();

	// Event callback for a visual highlight
	void _OnMouseMove (UIWidget* widget, const Vector2i& pos, const Vector2i& delta);
	void _OnFocusChange (UIWidget* widget, bool hasFocus) { OnFocus(false); }
	void _OnKeyPress  (UIWidget* widget, const Vector2i& pos, byte key, bool isDown);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIContext", UIContext, UIAnimatedFrame, UIWidget);

	// If alpha is set to '1', rebuild the entry list
	virtual void SetAlpha (float val, float animTime = 0.0f);
	virtual void OnFocus  (bool hasFocus);
};