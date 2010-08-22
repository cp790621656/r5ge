#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Updates the internal widgets' regions
//============================================================================================================

void UIColorPicker::UpdateRegions()
{
	float a0 = (mSliderWidth > 0) ? -(2.0f + mSliderWidth * 2.0f) : 0.0f;
	float a1 = (mSliderWidth > 0) ? (a0 + 1.0f) : 0.0f;
	float b0 = (mSliderWidth > 0) ? -(1.0f + mSliderWidth) : 0.0f;
	float b1 = (mSliderWidth > 0) ? (b0 + 1.0f) : 0.0f;

	mPicture.GetRegion().SetRight(1.0f, a0);
	mLuminance.GetRegion().SetLeft(1.0f, a1);
	mLuminance.GetRegion().SetRight(1.0f, b0);
	mAlpha.GetRegion().SetLeft(1.0f, b1);
	SetDirty();
}

//============================================================================================================
// Updates the background color using the current luminance
//============================================================================================================

void UIColorPicker::UpdateBackColor()
{
	float f = mLuminance.GetValue();
	byte a = Float::ToRangeByte(f);
	mPicture.SetBackColor(Color4ub(a, a, a, 255));
}

//============================================================================================================
// Gets the color at the specified mouse position
//============================================================================================================

Color4ub UIColorPicker::GetColor (Vector2i pos)
{
	const UIRegion& rgn = mPicture.GetRegion();
	float x = (pos.x - rgn.GetCalculatedLeft()) / rgn.GetCalculatedWidth();
	float y = (rgn.GetCalculatedBottom() - pos.y) / rgn.GetCalculatedHeight();

	x = Float::Clamp(x, 0.0f, 1.0f);
	y = Float::Clamp(y, 0.0f, 1.0f);

	return GetColor(x, y);
}

//============================================================================================================
// Gets the color at the specified relative position
//============================================================================================================

Color4f UIColorPicker::GetColor (float x, float y)
{
	float r = Float::Clamp(Float::Max(1.0f - x * 3.0f, 1.0f - (1.0f - x) * 3.0f), 0.0f, 1.0f);
	float g = Float::Clamp(1.0f - Float::Abs(1.0f / 3.0f - x) * 3.0f, 0.0f, 1.0f);
	float b = Float::Clamp(1.0f - Float::Abs(2.0f / 3.0f - x) * 3.0f, 0.0f, 1.0f);

	Color4f color (r, g, b);
	color.r = Interpolation::Linear(color.r, 1.0f, y);
	color.g = Interpolation::Linear(color.g, 1.0f, y);
	color.b = Interpolation::Linear(color.b, 1.0f, y);
	return color;
}

//============================================================================================================
// Sets the raw color by replacing its RGB with GetColor-returned values
//============================================================================================================

void UIColorPicker::SetColor (Vector2i pos)
{
	Color4ub c (GetColor(pos));
	c.a = mRawColor.a;
	mRawColor = c;
	OnValueChange();
}

//============================================================================================================
// Returns the current color selected by the widget
//============================================================================================================

Color4ub UIColorPicker::GetColor() const
{
	float f = mLuminance.GetValue();
	Color4f c (mRawColor);
	c.r *= f;
	c.g *= f;
	c.b *= f;
	c.a = mAlpha.GetValue();
	return c;
}

//============================================================================================================
// Sets the widget's color
//============================================================================================================

void UIColorPicker::SetColor (const Color4ub& color)
{
	Color4f c (color);
	mAlpha.SetValue(c.a);
	mLuminance.SetValue(c.Normalize());
	mRawColor = c;
	UpdateBackColor();
	OnValueChange();
}

//============================================================================================================
// Sets the width of the alpha and luminance sliders
//============================================================================================================

void UIColorPicker::SetSliderWidth (byte val)
{
	if (mSliderWidth != val)
	{
		mSliderWidth = val;
		UpdateRegions();
	}
}

//============================================================================================================
// Sets the UI root -- must be forwarded to the hidden children
//============================================================================================================

void UIColorPicker::_SetRootPtr (UIManager* ptr)
{
	UIWidget::_SetRootPtr(ptr);
	mPicture._SetRootPtr(ptr);
	mLuminance._SetRootPtr(ptr);
	mAlpha._SetRootPtr(ptr);
}

//============================================================================================================
// Initialize internal members and create the color texture
//============================================================================================================

void UIColorPicker::OnInit()
{
	ITexture* tex = mUI->GetTexture("UIColorPicker Texture");

	if (!tex->IsValid())
	{
		Array<Color4ub> colors;
		uint size = 128;
		colors.Reserve(size * size);

		float inv = 1.0f / size;

		for (uint y = 0; y < size; ++y)
		{
			float iy = inv * y;

			for (uint x = 0; x < size; ++x)
			{
				colors.Expand() = GetColor(x * inv, iy);
			}
		}

		// Set the texture's colors
		tex->SetWrapMode(ITexture::WrapMode::ClampToEdge);
		tex->Set(colors.GetBuffer(), size, size, 1, ITexture::Format::RGBA, ITexture::Format::RGB);
	}

	mPicture._SetParentPtr(this);
	mLuminance._SetParentPtr(this);
	mAlpha._SetParentPtr(this);

	mPicture.SetTexture(tex);
	mLuminance.SetValue(1.0f);
	mAlpha.SetValue(1.0f);

	UpdateRegions();
}

//============================================================================================================
// Texture has changed notification
//============================================================================================================

void UIColorPicker::OnTextureChanged (const ITexture* ptr)
{
	mLuminance.OnTextureChanged(ptr);
	mAlpha.OnTextureChanged(ptr);
}

//============================================================================================================
// Changing the layer should update hidden children as well
//============================================================================================================

void UIColorPicker::OnLayerChanged()
{
	mPicture.SetLayer(mLayer);
	mLuminance.SetLayer(mLayer);
	mAlpha.SetLayer(mLayer);
}

//============================================================================================================
// Update the widget's dimensions
//============================================================================================================

bool UIColorPicker::OnUpdate (bool dimensionsChanged)
{
	bool retVal = false;
	retVal |= mPicture.Update(mRegion, dimensionsChanged, true);
	retVal |= mLuminance.Update(mRegion, dimensionsChanged, true);
	retVal |= mAlpha.Update(mRegion, dimensionsChanged, true);
	return retVal;
}

//============================================================================================================
// Fill the draw queue
//============================================================================================================

void UIColorPicker::OnFill (UIQueue* queue)
{
	mPicture.OnFill(queue);
	mLuminance.OnFill(queue);
	mAlpha.OnFill(queue);
}

//============================================================================================================
// Key event notification
//============================================================================================================

void UIColorPicker::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	mFocus = 0;

	if (key == Key::MouseLeft && isDown)
	{
		if (mPicture.GetRegion().Contains(pos))
		{
			mFocus = 1;
			SetColor(pos);
		}
		else if (mLuminance.GetRegion().Contains(pos))
		{
			mFocus = 2;
			mLuminance.OnKeyPress(pos, key, isDown);
			UpdateBackColor();
			OnValueChange();
		}
		else if (mAlpha.GetRegion().Contains(pos))
		{
			mFocus = 3;
			mAlpha.OnKeyPress(pos, key, isDown);
			OnValueChange();
		}
	}
}

//============================================================================================================
// Mouse movement notification
//============================================================================================================

void UIColorPicker::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mFocus == 1)
	{
		SetColor(pos);
	}
	else if (mFocus == 2)
	{
		mLuminance.OnMouseMove(pos, delta);
		UpdateBackColor();
		OnValueChange();
	}
	else if (mFocus == 3)
	{
		mAlpha.OnMouseMove(pos, delta);
		OnValueChange();
	}
	UIWidget::OnMouseMove(pos, delta);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIColorPicker::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Color", mRawColor);
	const UISkin* skin = GetSkin();
	if (skin != 0 && skin != mUI->GetDefaultSkin()) root.AddChild("Skin", skin->GetName());
	root.AddChild("Slider Width", mSliderWidth);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIColorPicker::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Color")
	{
		Color4ub c;
		if (node.mValue >> c) SetColor(c);
		return true;
	}
	else if (node.mTag == "Skin")
	{
		SetSkin( mUI->GetSkin(node.mValue.AsString()) );
		return true;
	}
	else if (node.mTag == "Slider Width")
	{
		uint width;
		if (node.mValue >> width) SetSliderWidth((byte)width);
		return true;
	}
	return false;
}