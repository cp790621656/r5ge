#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIButton::UIButton() :
	mPrefix			(ClassID()),
	mState			(State::Enabled),
	mSticky			(false),
	mIgnoreMouseKey	(false),
	mPadding		(0)
{
	mLabel.SetAlignment(UILabel::Alignment::Center);
	mImage.SetLayer(0, false);
	mLabel.SetLayer(1, false);
}

//============================================================================================================
// Changes the padding on the sides of the internal text label
//============================================================================================================

void UIButton::SetTextPadding (int padding)
{
	if (mPadding != padding)
	{
		mPadding = padding;

		UIRegion& rgn = mLabel.GetRegion();
		rgn.SetLeft	 (0.0f,  (float)padding);
		rgn.SetRight (1.0f, -(float)padding);
		rgn.SetTop	 (0.0f,  (float)padding);
		rgn.SetBottom(1.0f, -(float)padding);
	}
}

//============================================================================================================
// Changes the button's state by changing the background face
//============================================================================================================

bool UIButton::SetState (uint state, bool val)
{
	uint newState = val ? (mState | state) : (mState & ~state);

	if (mState != newState)
	{
		if ((state & State::Pressed) != 0 && !val) mIgnoreMouseKey = false;

		mState = newState;

		if (GetSkin() == 0) SetSkin(mUI->GetDefaultSkin());

		const ITexture* tex = mImage.GetTexture();
		if (tex != 0) OnDirty(tex);

		OnStateChange(state, val);
		return true;
	}
	return false;
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void UIButton::_SetParentPtr (UIWidget* ptr)
{
	UIWidget::_SetParentPtr(ptr);
	mImage._SetParentPtr(this);
	mLabel._SetParentPtr(this);
}

//============================================================================================================

void UIButton::_SetRootPtr (UIManager* ptr)
{
	UIWidget::_SetRootPtr(ptr);
	mImage._SetRootPtr(ptr);
	mLabel._SetRootPtr(ptr);
}

//============================================================================================================
// Marking the button as dirty should mark the associated texture as dirty regardless of 'face' being set.
//============================================================================================================

void UIButton::SetDirty()
{
	const ITexture* tex = mImage.GetTexture();
	if (tex != 0) OnDirty(tex);
	mLabel.SetDirty();
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIButton::OnUpdate (bool dimensionsChanged)
{
	// Use the default face for the sake of calculating proper dimensions
	if (mImage.GetFace() == 0) mImage.SetFace(mPrefix + ": Enabled", false);
	dimensionsChanged |= mImage.Update(mRegion, dimensionsChanged, true);
	mLabel.Update(mImage.GetSubRegion(), dimensionsChanged, true);
	return false;
}

//============================================================================================================
// Fills the rendering queue
//============================================================================================================

void UIButton::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mWidget == 0 && queue->mTex == mImage.GetTexture())
	{
		if (mState & State::Enabled)
		{
			mImage.SetFace(mPrefix + ": Enabled", false);
			mImage.OnFill(queue);

			if (mState & State::Highlighted)
			{
				mImage.SetFace(mPrefix + ": Highlighted", false);
				mImage.OnFill(queue);
			}

			if (mState & State::Pressed)
			{
				mImage.SetFace(mPrefix + ": Pressed", false);
				mImage.OnFill(queue);
			}
		}
		else
		{
			mImage.SetFace(mPrefix + ": Disabled", false);
			mImage.OnFill(queue);
		}
	}
	else mLabel.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIButton::OnSerializeFrom (const TreeNode& node)
{
	if (mImage.OnSerializeFrom(node))
	{
		return true;
	}
	else if (node.mTag == "Prefix")
	{
		mPrefix = node.mValue.AsString();
		return true;
	}
	else if (node.mTag == "Text Padding")
	{
		int padding (0);
		if (node.mValue >> padding) SetTextPadding(padding);
		return true;
	}
	else if (node.mTag == "State")
	{
		if (node.mValue.IsString())
		{
			const String& state = node.mValue.AsString();

			if (state == "Disabled")
			{
				SetState(State::Enabled, false);
				SetState(State::Pressed, false);
			}
			else if (state == "Pressed")
			{
				SetState(State::Enabled, true);
				SetState(State::Pressed, true);
				SetSticky(true);
			}
			else if (state == "Enabled")
			{
				SetState(State::Enabled, true);
				SetState(State::Pressed, false);
			}
		}
		return true;
	}
	else if ( node.mTag == "Sticky" )
	{
		bool val;
		if (node.mValue >> val) SetSticky(val);
		return true;
	}
	return mLabel.OnSerializeFrom (node);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIButton::OnSerializeTo (TreeNode& node) const
{
	// Only the skin is saved from the SubPicture. Face is ignored.
	const UISkin* skin = mImage.GetSkin();

	// Only save the skin if it's something other than the default one
	if (skin != 0 && skin != mUI->GetDefaultSkin())
		node.AddChild("Skin", skin->GetName());

	// Add the optional prefix if it's different from its default value
	if (mPrefix != ClassID()) node.AddChild("Prefix", mPrefix);

	// Save the background color
	node.AddChild("Back Color", mImage.GetBackColor());

	// Padding around the text label
	if (mPadding != 0) node.AddChild("Text Padding", mPadding);

	// Label settings are saved fully
	mLabel.OnSerializeTo(node);

	// Save the state
	if		(mState & State::Pressed && mSticky)	node.AddChild("State", "Pressed");
	else if (mState & State::Enabled)				node.AddChild("State", "Enabled");
	else											node.AddChild("State", "Disabled");

	// Whether the button is a sticky (push) button
	node.AddChild("Sticky", mSticky);
}

//============================================================================================================
// Event handling -- mouse hovering over the button should highlight it
//============================================================================================================

void UIButton::OnMouseOver (bool inside)
{
	if (mState & State::Enabled)
	{
		SetState (State::Highlighted, inside);
		UIWidget::OnMouseOver(inside);
	}
}

//============================================================================================================
// Event handling -- mouse movement should be intercepted
//============================================================================================================

void UIButton::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	if (mState & State::Enabled)
	{
		UIWidget::OnMouseMove(pos, delta);
	}
}

//============================================================================================================
// Event handling -- left mouse button presses the button
//============================================================================================================

void UIButton::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (mState & State::Enabled)
	{
		if (key == Key::MouseLeft)
		{
			if (isDown)
			{
				// If we're ignoring this key, don't continue but don't ignore the next one (mouse up)
				if (mIgnoreMouseKey)
				{
					mIgnoreMouseKey = false;
					return;
				}

				// If this is a sticky button and it's not currently pressed, ignore the next mouse key
				mIgnoreMouseKey = (mSticky && !(mState & State::Pressed));

				// Update the button's state to be pressed
				SetState(State::Pressed, true);
			}
			else
			{
				if (mRegion.Contains(pos))
				{
					// If we should ignore this key, don't continue
					if (mIgnoreMouseKey) return;

					// Otherwise set the button's state to be not pressed and continue
					SetState(State::Pressed, false);
				}
				else
				{
					// Don't ignore the next key
					mIgnoreMouseKey = false;

					// Not in region - the button is not pressed or highlighted
					SetState(State::Highlighted, false);
					SetState(State::Pressed, false);
				}
			}
		}
		UIWidget::OnKeyPress(pos, key, isDown);
	}
}