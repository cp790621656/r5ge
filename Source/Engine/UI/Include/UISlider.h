#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Slider
// Author: Michael Lyashenko
//============================================================================================================

class UISlider : public UIWidget
{
protected:

	mutable UISkin*	mSkin;

	UIFace*		mFull;
	UIFace*		mEmpty;
	UIFace*		mKnob;
	float		mVal;
	Color4ub	mColor;
	String		mPrefix;

public:

	UISlider() : mSkin(0), mFull(0), mEmpty(0), mKnob(0), mVal(0.0f), mColor(0xFFFFFFFF), mPrefix(ClassID()) {}

	const ITexture* GetTexture()	const;
	virtual float	GetValue()		const	{ return mVal;   }
	const UISkin*	GetSkin()		const	{ return mSkin;  }
	const Color4ub&	GetBackColor()	const	{ return mColor; }

	// Sets the slider's value directly
	virtual void SetValue (float val);

	// Sets the slider's value by the 2D position
	void SetValue		(const Vector2i& pos);
	void SetSkin		(const UISkin* skin, bool setDirty = true);
	void SetBackColor	(const Color4ub& color);
	void SetPrefix		(const String& prefix);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UISlider", UISlider, UIWidget, UIWidget);

	// Marks this specific widget as needing to be rebuilt
	virtual void SetDirty() { const ITexture* tex = GetTexture(); if (tex) OnDirty(tex); }

	// Called when something changes in the skin
	virtual void OnTextureChanged (const ITexture* ptr);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;

	// Events
	virtual void OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual void OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
	virtual void OnFocus	(bool selected)	{ UIWidget::OnFocus(selected); }
};