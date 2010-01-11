#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple textured quad
//============================================================================================================

class UIPicture : public UIArea
{
protected:

	const ITexture*	mTex;
	bool			mIgnoreAlpha;

public:

	UIPicture() : mTex(0), mIgnoreAlpha(false) {}

	const ITexture* GetTexture() const { return mTex; }
	void SetTexture (const ITexture* tex);
	bool IsIgnoringAlpha() const { return mIgnoreAlpha; }
	void IgnoreAlpha (bool val) { if (mIgnoreAlpha != val) { mIgnoreAlpha = val; SetDirty(); } }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Picture", UIPicture, UIArea, UIArea);

protected:

	// Marks this specific area as needing to be rebuilt
	virtual void SetDirty() { if (mTex) OnDirty(mTex); }

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;
};