#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// UIPicture containing a color gradient and 2 sliders (luminance, alpha)
//============================================================================================================

class UIColorPicker : public UIWidget
{
protected:

	Color4ub	mRawColor;
	UIPicture	mPicture;
	UISlider	mLuminance;
	UISlider	mAlpha;
	byte		mSliderWidth;
	byte		mFocus;

	UIColorPicker() : mRawColor(0xFFFFFFFF), mSliderWidth(10), mFocus(0) {}

	// Updates the internal widgets' regions
	void UpdateRegions();

	// Gets the color at the specified mouse position
	Color4ub GetColor (Vector2i pos);

	// Gets the color at the specified relative position
	Color4ub GetColor (float x, float y);

	// Sets the raw color by replacing its RGB with GetColor-returned values
	void SetColor (Vector2i pos);

public:

	R5_DECLARE_INHERITED_CLASS("UIColorPicker", UIColorPicker, UIWidget, UIWidget);

	// Gets the skin used by the sliders
	const UISkin* GetSkin() const { return mLuminance.GetSkin(); }

	// Sets the skin used by the sliders
	void SetSkin (const UISkin* skin) { mLuminance.SetSkin(skin); mAlpha.SetSkin(skin); }

	// Returns the current color selected by the widget
	Color4ub GetColor() const;

	// Sets the widget's color
	void SetColor (const Color4ub& color);

	// The width of the alpha and luminance sliders
	byte GetSliderWidth() const { return mSliderWidth; }

	// Sets the width of the alpha and luminance sliders
	void SetSliderWidth (byte val);

protected:

	// Sets the UI root -- must be forwarded to the hidden children
	virtual void _SetRootPtr (UIManager* ptr);

public:

	// Initialize internal members and create the color texture
	virtual void OnInit();

	// Texture has changed notification
	virtual void OnTextureChanged(const ITexture* ptr);

	// Changing the layer should update hidden children as well
	virtual void OnLayerChanged();

	// Marks the widget as needing to be rebuilt
	virtual void SetDirty() { mPicture.SetDirty(); mLuminance.SetDirty(); }

	// Update the widget's dimensions
	virtual bool OnUpdate (bool dimensionsChanged);

	// Fill the draw queue
	virtual void OnFill (UIQueue* queue);

	// Key event notification
	virtual void OnKeyPress(const Vector2i& pos, byte key, bool isDown);

	// Mouse movement notification
	virtual void OnMouseMove (const Vector2i& pos, const Vector2i& delta);

	// Serialization
	virtual void OnSerializeTo (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};