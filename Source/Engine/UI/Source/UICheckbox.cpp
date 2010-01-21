#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UICheckbox::OnUpdate (bool dimensionsChanged)
{
	if (dimensionsChanged)
	{
		float height = mRegion.GetCalculatedHeight();

		if (GetAlignment() == UILabel::Alignment::Right)
		{
			mImage.GetRegion().SetLeft (1.0f, -height);
			mLabel.GetRegion().SetRight(1.0f, -height);
		}
		else
		{
			mImage.GetRegion().SetRight(0.0f, height);
			mLabel.GetRegion().SetLeft (0.0f, height);
		}
	}
	mImage.Update(mRegion, dimensionsChanged);
	mLabel.Update(mRegion, dimensionsChanged);
	return false;
}

//============================================================================================================
// Fills the rendering queue
//============================================================================================================

void UICheckbox::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mWidget == 0)
	{
		if (queue->mTex != 0 && queue->mTex == mImage.GetTexture())
		{
			if (mState & State::Enabled)
			{
				mImage.SetFace(mPrefix + ": Unchecked", false);
				mImage.OnFill(queue);

				if (mState & State::Highlighted)
				{
					mImage.SetFace(mPrefix + ": Highlighted", false);
					mImage.OnFill(queue);
				}

				if (mState & State::Checked)
				{
					mImage.SetFace(mPrefix + ": Checked", false);
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
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UICheckbox::OnSerializeFrom (const TreeNode& node)
{
	if ( mImage.OnSerializeFrom(node) )
	{
		return true;
	}
	else if (node.mTag == "Prefix")
	{
		mPrefix = node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString();
		return true;
	}
	else if (node.mTag == "State")
	{
		if (node.mValue.IsString())
		{
			const String& state = node.mValue.AsString();

			if		(state == "Disabled")	SetState(State::Enabled, false);
			else if (state == "Checked")	SetState(State::Enabled | State::Checked, true);
			else if (state == "Enabled")	SetState(State::Enabled, true);
		}
		return true;
	}
	return mLabel.OnSerializeFrom (node);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UICheckbox::OnSerializeTo (TreeNode& node) const
{
	// Only the skin is saved from the SubPicture. Face is ignored.
	const UISkin* skin = mImage.GetSkin();

	// Only save the skin if it's something other than the default one
	if (skin != 0 && skin != mUI->GetDefaultSkin())
		node.AddChild("Skin", skin->GetName());

	// Add the optional prefix if it's different from its default value
	if (mPrefix != ClassID()) node.AddChild("Prefix", mPrefix);

	mLabel.OnSerializeTo(node);

	if		(mState & State::Checked)	node.AddChild("State", "Checked");
	else if (mState & State::Enabled)	node.AddChild("State", "Enabled");
	else								node.AddChild("State", "Disabled");
}
