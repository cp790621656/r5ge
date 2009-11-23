#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
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
	Color3f			mColor;
	bool			mShadow;
	uint			mAlignment;
	Entries			mEntries;
	bool			mIsDirty;
	float			mMinWidth;
	Vector3f		mAnchor;
	Vector2f		mSize;
	String			mLast;

public:

	UIContext();

	const UISkin*	GetSkin()			const	{ return mSkin; }
	const IFont*	GetFont()			const	{ return mFont; }
	const Color3f&	GetColor()			const	{ return mColor; }
	bool			DropsShadow()		const	{ return mShadow;  }
	uint			GetAlignment()		const	{ return mAlignment; }
	const Entries&	GetAllEntries()		const	{ return mEntries; }
	float			GetMinWidth()		const	{ return mMinWidth; }
	const Vector3f&	GetAnchor()			const	{ return mAnchor; }
	const String&	GetLastSelection()	const	{ return mLast; }

	void SetSkin		(const UISkin* skin)	{ mSkin		 = skin; mIsDirty = true; }
	void SetFace		(const String& face)	{ mFace		 = face; mIsDirty = true; }
	void SetFont		(const IFont* font)		{ mFont		 = font; mIsDirty = true; }
	void SetColor		(const Color3f& c)		{ mColor	 = c;	 mIsDirty = true; }
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
	bool _OnMouseOverItem (UIArea* area, bool inside);
	bool _OnItemFocus (UIArea* area, bool hasFocus);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Context", UIContext, UIAnimatedFrame, UIArea);

	// If alpha is set to '1', rebuild the entry list
	virtual void SetAlpha (float val, float animTime = 0.0f);

	// Respond to selection and ignore keys and mouse movement
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ return true; }
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown)	{ return true; }
	virtual bool OnFocus	(bool hasFocus);
};