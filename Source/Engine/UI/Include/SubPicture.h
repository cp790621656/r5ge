#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Drawable Face
//============================================================================================================

class SubPicture : public Area
{
protected:

	Skin*	mSkin;		// Pointer to the skin used to draw this area
	Face*	mFace;		// Pointer to the face within the skin used to draw this area
	short	mBorder;	// Saved border value, used to track face border changes
	Region	mSubRegion;	// Child region, calculated based on the border

public:

	SubPicture() : mSkin(0), mFace(0), mBorder(0) {}

	const ITexture* GetTexture() const	{ return mSkin ? mSkin->GetTexture() : 0; }
	const Skin*		GetSkin()	 const	{ return mSkin; }
	const Face*		GetFace()	 const	{ return mFace; }

	void Set	 (const Skin* skin, const String& face, bool setDirty = true);
	void SetSkin (const Skin* skin, bool setDirty = true);
	void SetFace (const String& face, bool setDirty = true);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("SubPicture", SubPicture, Area, Area);

	// Area functions
	virtual const Region& GetSubRegion() const { return mSubRegion; }
	virtual void SetDirty() { const ITexture* tex = GetTexture(); if (tex) OnDirty(tex); }
	virtual void OnTextureChanged (const ITexture* ptr);
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};