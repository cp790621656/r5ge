#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Button::Button() : mState(State::Enabled), mSticky(false), mIgnoreMouseKey(false)
{
	mLabel.SetAlignment(Label::Alignment::Center);
	mImage.SetLayer(0, false);
	mLabel.SetLayer(1, false);
}

//============================================================================================================
// Changes the button's state by changing the background face
//============================================================================================================

bool Button::SetState (uint state, bool val)
{
	uint newState ( val ? (mState | state) : (mState & ~state) );

	if ( mState != newState )
	{
		if ( state == State::Pressed && val == false )
		{
			mIgnoreMouseKey = false;
		}

		mState = newState;
		if (mImage.GetSkin() != 0) mImage.SetDirty();
		if (mOnStateChange) mOnStateChange(this);
		return true;
	}
	return false;
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void Button::_SetParentPtr (Area* ptr)
{
	Area::_SetParentPtr(ptr);
	mImage._SetParentPtr(this);
	mLabel._SetParentPtr(this);
}

//============================================================================================================

void Button::_SetRootPtr (Root* ptr)
{
	Area::_SetRootPtr(ptr);
	mImage._SetRootPtr(ptr);
	mLabel._SetRootPtr(ptr);
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool Button::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= mImage.Update(mRegion, dimensionsChanged);
	mLabel.Update(mImage.GetSubRegion(), dimensionsChanged);
	return false;
}

//============================================================================================================
// Fills the rendering queue
//============================================================================================================

void Button::OnFill (Queue* queue)
{
	if (queue->mLayer	== mLayer &&
		queue->mTex	== mImage.GetTexture() &&
		queue->mArea	== 0)
	{
		static String faceName[] =
		{
			"Button: Disabled",
			"Button: Enabled",
			"Button: Highlighted",
			"Button: Pressed"
		};

		if (mState & State::Enabled)
		{
			mImage.SetFace(faceName[1], false);
			mImage.OnFill(queue);

			if (mState & State::Highlighted)
			{
				mImage.SetFace(faceName[2], false);
				mImage.OnFill(queue);
			}

			if (mState & State::Pressed)
			{
				mImage.SetFace(faceName[3], false);
				mImage.OnFill(queue);
			}
		}
		else
		{
			mImage.SetFace(faceName[0], false);
			mImage.OnFill(queue);
		}
	}
	else mLabel.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Button::CustomSerializeFrom (const TreeNode& root)
{
	if ( mImage.CustomSerializeFrom(root) )
	{
		return true;
	}
	else if (root.mTag == "State")
	{
		if (root.mValue.IsString())
		{
			const String& state = root.mValue.AsString();

			if		(state == "Disabled")	SetState(State::Enabled, false);
			else if (state == "Pressed")	SetState(State::Enabled | State::Pressed, true);
			else if (state == "Enabled")	SetState(State::Enabled, true);
		}
		return true;
	}
	else if ( root.mTag == "Sticky" )
	{
		bool val;
		if (root.mValue >> val) SetSticky(val);
		return true;
	}
	return mLabel.CustomSerializeFrom(root);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Button::CustomSerializeTo (TreeNode& root) const
{
	// Only the skin is saved from the SubPicture. Face is ignored.
	const Skin* skin = mImage.GetSkin();
	TreeNode& child = root.AddChild("Skin");
	if (skin) child.mValue = skin->GetName();

	// Label settings are saved fully
	mLabel.CustomSerializeTo(root);

	// Save the state
	if		(mState & State::Pressed && mSticky)	root.AddChild("State", "Pressed");
	else if (mState & State::Enabled)				root.AddChild("State", "Enabled");
	else											root.AddChild("State", "Disabled");

	// Whether the button is a sticky (push) button
	root.AddChild("Sticky", mSticky);
}

//============================================================================================================
// Event handling -- mouse hovering over the button should highlight it
//============================================================================================================

bool Button::OnMouseOver (bool inside)
{
	if (mState & State::Enabled)
	{
		SetState (State::Highlighted, inside);
		Area::OnMouseOver(inside);
	}
	return true;
}

//============================================================================================================
// Event handling -- mouse movement should be intercepted
//============================================================================================================

bool Button::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	if (mState & State::Enabled) Area::OnMouseMove(pos, delta);
	return true;
}

//============================================================================================================
// Event handling -- left mouse button presses the button
//============================================================================================================

bool Button::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	bool retVal = false;

	if (mState & State::Enabled)
	{
		if (key == Key::MouseLeft)
		{
			retVal = true;

			if (isDown)
			{
				// If we're ignoring this key, don't continue but don't ignore the next one (mouse up)
				if (mIgnoreMouseKey)
				{
					mIgnoreMouseKey = false;
					return true;
				}

				// If this is a sticky button and it's not currently pressed, ignore the next mouse key
				mIgnoreMouseKey = (mSticky && !(mState & State::Pressed));

				// Update the button's state to be pressed and highlighted
				SetState(State::Pressed | State::Highlighted, true);
			}
			else
			{
				if (mRegion.Contains(pos))
				{
					// If we should ignore this key, don't continue
					if (mIgnoreMouseKey) return true;

					// Otherwise set the button's state to be not pressed and continue
					SetState(State::Pressed, false);
				}
				else
				{
					// Don't ignore the next key
					mIgnoreMouseKey = false;

					// Not in region - the button is not pressed or highlighted
					SetState(State::Pressed | State::Highlighted, false);
				}
			}
		}
		Area::OnKey(pos, key, isDown);
	}
	return retVal;
}