#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Simple textured quad
//============================================================================================================

class Picture : public Area
{
protected:

	const ITexture*	mTex;
	bool			mIgnoreAlpha;

public:

	Picture() : mTex(0), mIgnoreAlpha(false) {}

	const ITexture* GetTexture() const { return mTex; }
	void SetTexture (const ITexture* tex);
	bool IsIgnoringAlpha() const { return mIgnoreAlpha; }
	void IgnoreAlpha (bool val) { if (mIgnoreAlpha != val) { mIgnoreAlpha = val; SetDirty(); } }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Picture", Picture, Area, Area);

protected:

	// Marks this specific area as needing to be rebuilt
	virtual void SetDirty() { if (mTex) OnDirty(mTex); }

	// Called when a queue is being rebuilt
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};