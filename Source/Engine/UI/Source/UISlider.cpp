#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves the texture associated with the widget
//============================================================================================================

const ITexture* UISlider::GetTexture() const
{
	if (mSkin == 0) const_cast<UISlider*>(this)->SetSkin(mUI->GetDefaultSkin(), false);
	return (mSkin != 0) ? mSkin->GetTexture() : 0;
}

//============================================================================================================
// Changes the slider's value
//============================================================================================================

void UISlider::SetValue (float val)
{
	val = Float::Clamp(val, 0.0f, 1.0f);

	if ( Float::IsNotEqual(mVal, val) )
	{
		mVal = val;
		OnValueChange();
		SetDirty();
	}
}

//============================================================================================================
// Sets the slider's value by the 2D position
//============================================================================================================

void UISlider::SetValue (const Vector2i& pos)
{
	float left	 ( mRegion.GetCalculatedLeft()   );
	float bottom ( mRegion.GetCalculatedBottom() );
	float width  ( mRegion.GetCalculatedWidth()  );
	float height ( mRegion.GetCalculatedHeight() );

	if (width > height)
	{
		SetValue( (pos.x - left) / width );
	}
	else
	{
		SetValue( (bottom - pos.y) / height );
	}
}

//============================================================================================================
// Changes the skin
//============================================================================================================

void UISlider::SetSkin (const UISkin* skin, bool setDirty)
{
	if (mSkin != skin)
	{
		if (setDirty) SetDirty();
		mSkin	= const_cast<UISkin*>(skin);
		mFull	= mSkin->GetFace(mPrefix + ": Full" );
		mEmpty	= mSkin->GetFace(mPrefix + ": Empty");
		mKnob	= mSkin->GetFace(mPrefix + ": Knob" );
		if (setDirty) SetDirty();
	}
}

//============================================================================================================
// Changes the color
//============================================================================================================

void UISlider::SetBackColor (const Color4ub& color)
{
	if (mColor != color)
	{
		mColor = color;
		SetDirty();
	}
}

//============================================================================================================
// Sets the prefix used by the slider
//============================================================================================================

void UISlider::SetPrefix (const String& prefix)
{
	if (mPrefix != prefix)
	{
		mPrefix = prefix;

		if (mSkin != 0)
		{
			mFull	= mSkin->GetFace(mPrefix + ": Full" );
			mEmpty	= mSkin->GetFace(mPrefix + ": Empty");
			mKnob	= mSkin->GetFace(mPrefix + ": Knob" );
			SetDirty();
		}
	}
}

//============================================================================================================
// Called when something changes in the skin
//============================================================================================================

void UISlider::OnTextureChanged (const ITexture* ptr)
{
	const ITexture* tex = GetTexture();
	if (tex == ptr) OnDirty(tex);
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UISlider::OnFill (UIQueue* queue)
{
	if (queue->mLayer	== mLayer &&
		queue->mTex		!= 0 &&
		queue->mTex		== GetTexture() &&
		queue->mWidget	== 0)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetCalculatedLeft()   );
		float top	 ( mRegion.GetCalculatedTop()    );
		float right	 ( mRegion.GetCalculatedRight()  );
		float bottom ( mRegion.GetCalculatedBottom() );
		float width	 ( mRegion.GetCalculatedWidth()  );
		float height ( mRegion.GetCalculatedHeight() );

		Color4ub white	 ( 255, 255, 255, Float::ToRangeByte(mColor.GetAlpha() * mRegion.GetCalculatedAlpha()) );
		Color4ub color   ( mColor, mRegion.GetCalculatedAlpha() );
		Vector2i texSize ( queue->mTex->GetSize() );

		Rectangle<float>  full  (  mFull->GetRectangle(texSize) );
		Rectangle<float>  empty ( mEmpty->GetRectangle(texSize) );
		Rectangle<float>  knob  (  mKnob->GetRectangle(texSize) );

		if ( width > height )
		{
			// Pixel-aligned mVal-based offset
			float offset		= Float::Round(mVal * width);
			float factor		= offset / width;
			float invFactor		= 1.0f - factor;
			float centerFull	= left  + offset;
			float centerEmpty	= right - (width - offset);
			float knobLeft		= centerFull - Float::Round(height * 0.5f);
			float knobRight		= knobLeft + height;
			float tcCenterFull  =  full.left  + ( full.right -  full.left) * factor;
			float tcCenterEmpty = empty.right - (empty.right - empty.left) * invFactor;

			// Full bar
			v.Expand().Set( left,			top,    full.left,		full.top,		color );
			v.Expand().Set( left,			bottom, full.left,		full.bottom,	color );
			v.Expand().Set( centerFull,		bottom, tcCenterFull,	full.bottom,	color );
			v.Expand().Set( centerFull,		top,    tcCenterFull,	full.top,		color );

			// Empty bar
			v.Expand().Set( centerEmpty,	top,    tcCenterEmpty,	empty.top,		white );
			v.Expand().Set( centerEmpty,	bottom, tcCenterEmpty,	empty.bottom,	white );
			v.Expand().Set( right,			bottom, empty.right,	empty.bottom,	white );
			v.Expand().Set( right,			top,    empty.right,	empty.top,		white );

			// Knob
			v.Expand().Set( knobLeft,		top,	knob.left,		knob.top,		white);
			v.Expand().Set( knobLeft,		bottom, knob.left,		knob.bottom,	white);
			v.Expand().Set( knobRight,		bottom, knob.right,		knob.bottom,	white);
			v.Expand().Set( knobRight,		top,	knob.right,		knob.top,		white);
		}
		// Vertical slider
		else
		{
			// Pixel-aligned factor-based offset
			float offset		= Float::Round(mVal * height);
			float factor		= offset / height;
			float invFactor		= 1 - factor;
			float centerFull	= bottom - offset;
			float centerEmpty	= top + (height - offset);
			float knobTop		= centerFull - Float::Round(width * 0.5f);
			float knobBottom	= knobTop + width;
			float tcCenterFull  =  full.left  + ( full.right -  full.left) * factor;
			float tcCenterEmpty = empty.right - (empty.right - empty.left) * invFactor;

			// Full bar
			v.Expand().Set( left,	centerFull,		tcCenterFull,	full.top,		color);
			v.Expand().Set( left,	bottom,			full.left,		full.top,		color);
			v.Expand().Set( right,	bottom,			full.left,		full.bottom,	color);
			v.Expand().Set( right,	centerFull,		tcCenterFull,	full.bottom,	color);

			// Empty bar
			v.Expand().Set( left,	top,			empty.right,	empty.top,		white);
			v.Expand().Set( left,	centerEmpty,	tcCenterEmpty,	empty.top,		white);
			v.Expand().Set( right,	centerEmpty,	tcCenterEmpty,	empty.bottom,	white);
			v.Expand().Set( right,	top,			empty.right,	empty.bottom,	white);

			// Knob
			v.Expand().Set( left,	knobTop,		knob.right,		knob.top,		white);
			v.Expand().Set( left,	knobBottom,		knob.left,		knob.top,		white);
			v.Expand().Set( right,	knobBottom,		knob.left,		knob.bottom,	white);
			v.Expand().Set( right,	knobTop,		knob.right,		knob.bottom,	white);
		}
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UISlider::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;

	if (node.mTag == "Skin")
	{
		SetSkin( mUI->GetSkin( value.AsString() ) );
		return true;
	}
	else if (node.mTag == "Value")
	{
		float val;
		if (value >> val) SetValue(val);
		return true;
	}
	else if (node.mTag == "Back Color" || node.mTag == "Color")
	{
		Color4ub color;
		if (value >> color) SetBackColor(color);
		return true;
	}
	else if (node.mTag == "Prefix")
	{
		SetPrefix( value.AsString() );
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UISlider::OnSerializeTo (TreeNode& node) const
{
	if (mSkin != 0 && mSkin != mUI->GetDefaultSkin())
		node.AddChild("Skin", mSkin->GetName());

	node.AddChild("Value", mVal);
	node.AddChild("Back Color", mColor);

	// Add the optional prefix if it's different from its default value
	if (mPrefix != ClassID()) node.AddChild("Prefix", mPrefix);
}

//============================================================================================================
// Respond to mouse movement
//============================================================================================================

void UISlider::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if ( mUI->IsKeyDown(Key::MouseLeft) )
	{
		SetValue(pos);
	}
	UIWidget::OnMouseMove(pos, delta);
}

//============================================================================================================
// Respond to key events -- namely the left mouse button
//============================================================================================================

void UISlider::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if ( key == Key::MouseLeft )
	{
		if (isDown) SetValue(pos);
	}
	UIWidget::OnKeyPress(pos, key, isDown);
}