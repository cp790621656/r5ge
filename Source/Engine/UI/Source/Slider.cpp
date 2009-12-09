#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the slider's value
//============================================================================================================

void UISlider::SetValue (float val)
{
	val = Float::Clamp(val, 0.0f, 1.0f);

	if ( Float::IsNotEqual(mVal, val) )
	{
		mVal = val;
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

void UISlider::SetSkin (const UISkin* skin)
{
	if (mSkin != skin)
	{
		SetDirty();
		mSkin	= const_cast<UISkin*>(skin);
		mFull  = mSkin->GetFace( "Slider: Full"  );
		mEmpty = mSkin->GetFace( "Slider: Empty" );
		mKnob  = mSkin->GetFace( "Slider: Knob"  );
		SetDirty();
	}
}

//============================================================================================================
// Changes the color
//============================================================================================================

void UISlider::SetColor(const Color3f& color)
{
	if (mColor != color)
	{
		mColor = color;
		SetDirty();
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
		queue->mArea	== 0)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetCalculatedLeft()   );
		float top	 ( mRegion.GetCalculatedTop()    );
		float right	 ( mRegion.GetCalculatedRight()  );
		float bottom ( mRegion.GetCalculatedBottom() );
		float width	 ( mRegion.GetCalculatedWidth()  );
		float height ( mRegion.GetCalculatedHeight() );

		Color4ub white	 ( 255, 255, 255, Float::ToRangeByte(mRegion.GetCalculatedAlpha()) );
		Color4ub color   ( mColor, mRegion.GetCalculatedAlpha() );
		Vector2i texSize ( queue->mTex->GetSize() );

		UIFace::Rectangle  full  (  mFull->GetRectangle(texSize) );
		UIFace::Rectangle  empty ( mEmpty->GetRectangle(texSize) );
		UIFace::Rectangle  knob  (  mKnob->GetRectangle(texSize) );

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
			float tcCenterFull  =  full.mLeft  + ( full.mRight -  full.mLeft) * factor;
			float tcCenterEmpty = empty.mRight - (empty.mRight - empty.mLeft) * invFactor;

			// Full bar
			v.Expand().Set( left,			top,    full.mLeft,		full.mTop,		color );
			v.Expand().Set( left,			bottom, full.mLeft,		full.mBottom,	color );
			v.Expand().Set( centerFull,		bottom, tcCenterFull,	full.mBottom,	color );
			v.Expand().Set( centerFull,		top,    tcCenterFull,	full.mTop,		color );

			// Empty bar
			v.Expand().Set( centerEmpty,	top,    tcCenterEmpty,	empty.mTop,		white );
			v.Expand().Set( centerEmpty,	bottom, tcCenterEmpty,	empty.mBottom,	white );
			v.Expand().Set( right,			bottom, empty.mRight,	empty.mBottom,	white );
			v.Expand().Set( right,			top,    empty.mRight,	empty.mTop,		white );

			// Knob
			v.Expand().Set( knobLeft,		top,	knob.mLeft,		knob.mTop,		white);
			v.Expand().Set( knobLeft,		bottom, knob.mLeft,		knob.mBottom,	white);
			v.Expand().Set( knobRight,		bottom, knob.mRight,	knob.mBottom,	white);
			v.Expand().Set( knobRight,		top,	knob.mRight,	knob.mTop,		white);
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
			float tcCenterFull  = full.mBottom - (full.mBottom  - full.mTop ) * factor;
			float tcCenterEmpty = empty.mTop   + (empty.mBottom - empty.mTop) * invFactor;

			// Full bar
			v.Expand().Set( left,	centerFull,		full.mLeft,		tcCenterFull,	color);
			v.Expand().Set( left,	bottom,			full.mLeft,		full.mBottom,	color);
			v.Expand().Set( right,	bottom,			full.mRight,	full.mBottom,	color);
			v.Expand().Set( right,	centerFull,		full.mRight,	tcCenterFull,	color);

			// Empty bar
			v.Expand().Set( left,	top,			empty.mLeft,	empty.mTop,		color);
			v.Expand().Set( left,	centerEmpty,	empty.mLeft,	tcCenterEmpty,	color);
			v.Expand().Set( right,	centerEmpty,	empty.mRight,	tcCenterEmpty,	color);
			v.Expand().Set( right,	top,			empty.mRight,	empty.mTop,		color);

			// Knob
			v.Expand().Set( left,	knobTop,		knob.mLeft,		knob.mTop,		color);
			v.Expand().Set( left,	knobBottom,		knob.mLeft,		knob.mBottom,	color);
			v.Expand().Set( right,	knobBottom,		knob.mRight,	knob.mBottom,	color);
			v.Expand().Set( right,	knobTop,		knob.mRight,	knob.mTop,		color);
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
		SetSkin( mRoot->GetSkin( value.IsString() ? value.AsString() : value.GetString() ) );
		return true;
	}
	else if (node.mTag == "Value")
	{
		float val;
		if (value >> val)
		{
			SetValue(val);
			if (mOnValueChange) mOnValueChange(this);
		}
		return true;
	}
	else if (node.mTag == "Color")
	{
		Color3f color;
		if (value >> color) SetColor(color);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UISlider::OnSerializeTo (TreeNode& node) const
{
	TreeNode& skin = node.AddChild("Skin");
	if (mSkin != 0) skin.mValue = mSkin->GetName();
	node.AddChild("Value", mVal);
	node.AddChild("Color", mColor);
}

//============================================================================================================
// Respond to mouse movement
//============================================================================================================

bool UISlider::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if ( mRoot->IsKeyDown(Key::MouseLeft) )
	{
		SetValue(pos);
	}
	UIArea::OnMouseMove(pos, delta);
	return true;
}

//============================================================================================================
// Respond to key events -- namely the left mouse button
//============================================================================================================

bool UISlider::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	if ( key == Key::MouseLeft )
	{
		if (isDown) SetValue(pos);
		UIArea::OnKey(pos, key, isDown);
		return true;
	}
	return UIArea::OnKey(pos, key, isDown);
}