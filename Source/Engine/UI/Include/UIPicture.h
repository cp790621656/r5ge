#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Simple textured quad
// Author: Michael Lyashenko
//============================================================================================================

class UIPicture : public UIWidget
{
protected:

	const ITexture*	mTex;
	bool			mIgnoreAlpha;
	Color4ub		mColor;
	bool			mTiled;

public:

	UIPicture() : mTex(0), mIgnoreAlpha(false), mColor(0xFFFFFFFF), mTiled(false) {}

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIPicture", UIPicture, UIWidget, UIWidget);

	const ITexture* GetTexture() const { return mTex; }
	void SetTexture (const ITexture* tex);

	bool IsIgnoringAlpha() const { return mIgnoreAlpha; }
	void IgnoreAlpha (bool val) { if (mIgnoreAlpha != val) { mIgnoreAlpha = val; SetDirty(); } }

	const Color4ub& GetBackColor() const { return mColor; }
	void SetBackColor (const Color4ub& c) { if (mColor != c) { mColor = c; SetDirty(); } }

	// The texture can be tiled or stretched
	bool IsTiled() const { return mTiled; }
	void SetTiled(bool val)	 { if (mTiled != val) { mTiled = val; SetDirty(); } }

public:

	// Marks this specific widget as needing to be rebuilt
	virtual void SetDirty() { if (mTex) OnDirty(mTex); }

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};