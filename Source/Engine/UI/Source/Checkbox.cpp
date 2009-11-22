#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool Checkbox::OnUpdate (bool dimensionsChanged)
{
	if (dimensionsChanged)
	{
		float height = mRegion.GetHeight();

		if (GetAlignment() == Label::Alignment::Right)
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

void Checkbox::OnFill (Queue* queue)
{
	if (queue->mLayer	== mLayer &&
		queue->mTex	== mImage.GetTexture() &&
		queue->mArea	== 0)
	{
		static String faceName[] = {"Checkbox: Disabled", "Checkbox: Unchecked", "Checkbox: Highlighted", "Checkbox: Checked"};

		if (mState & State::Enabled)
		{
			mImage.SetFace(faceName[1], false);
			mImage.OnFill(queue);

			if (mState & State::Highlighted)
			{
				mImage.SetFace(faceName[2], false);
				mImage.OnFill(queue);
			}

			if (mState & State::Checked)
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

bool Checkbox::CustomSerializeFrom (const TreeNode& root)
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
			else if (state == "Checked")	SetState(State::Enabled | State::Checked, true);
			else if (state == "Enabled")	SetState(State::Enabled, true);
		}
		return true;
	}
	return mLabel.CustomSerializeFrom(root);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Checkbox::CustomSerializeTo (TreeNode& root) const
{
	const Skin* skin = mImage.GetSkin();
	TreeNode& child = root.AddChild("Skin");
	if (skin) child.mValue = skin->GetName();

	mLabel.CustomSerializeTo(root);

	if		(mState & State::Checked)	root.AddChild("State", "Checked");
	else if (mState & State::Enabled)	root.AddChild("State", "Enabled");
	else								root.AddChild("State", "Disabled");
}
