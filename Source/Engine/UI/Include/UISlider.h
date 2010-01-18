#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Slider
//============================================================================================================

class UISlider : public UIWidget
{
protected:

	mutable UISkin*	mSkin;

	UIFace*		mFull;
	UIFace*		mEmpty;
	UIFace*		mKnob;
	float		mVal;
	Color3f		mColor;

public:

	UISlider() : mSkin(0), mFull(0), mEmpty(0), mKnob(0), mVal(0.0f), mColor(1.0f) {}

	const ITexture* GetTexture() const;
	virtual float	GetValue()	 const	{ return mVal;   }
	const UISkin*	GetSkin()	 const	{ return mSkin;  }
	const Color3f&	GetColor()	 const	{ return mColor; }

	// Sets the slider's value directly
	virtual void SetValue (float val);

	// Sets the slider's value by the 2D position
	void SetValue (const Vector2i& pos);
	void SetSkin  (const UISkin* skin, bool setDirty = true);
	void SetColor (const Color3f& color);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Slider", UISlider, UIWidget, UIWidget);

	// Marks this specific widget as needing to be rebuilt
	virtual void SetDirty() { const ITexture* tex = GetTexture(); if (tex) OnDirty(tex); }

	// Called when something changes in the skin
	virtual void OnTextureChanged (const ITexture* ptr);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;

	// Events
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnFocus	(bool selected)	{ UIWidget::OnFocus(selected); return true; }
};