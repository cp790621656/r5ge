#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Drawable Face
//============================================================================================================

class UISubPicture : public UIArea
{
protected:

	UISkin*		mSkin;		// Pointer to the skin used to draw this area
	UIFace*		mFace;		// Pointer to the face within the skin used to draw this area
	short		mBorder;	// Saved border value, used to track face border changes
	UIRegion	mSubRegion;	// Child region, calculated based on the border

public:

	UISubPicture() : mSkin(0), mFace(0), mBorder(0) {}

	const ITexture* GetTexture() const	{ return mSkin ? mSkin->GetTexture() : 0; }
	const UISkin*	GetSkin()	 const	{ return mSkin; }
	const UIFace*		GetFace()	 const	{ return mFace; }

	void Set	 (const UISkin* skin, const String& face, bool setDirty = true);
	void SetSkin (const UISkin* skin, bool setDirty = true);
	void SetFace (const String& face, bool setDirty = true);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("SubPicture", UISubPicture, UIArea, UIArea);

	// Area functions
	virtual const UIRegion& GetSubRegion() const { return mSubRegion; }
	virtual void SetDirty() { const ITexture* tex = GetTexture(); if (tex) OnDirty(tex); }
	virtual void OnTextureChanged (const ITexture* ptr);
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};