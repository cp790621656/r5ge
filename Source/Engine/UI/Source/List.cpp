#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// List selection should update the label
//============================================================================================================

void UIList::_OnValue (const String& val)
{
	mLabel.SetText(val);
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void UIList::_SetParentPtr (UIArea* ptr)
{
	UIMenu::_SetParentPtr(ptr);
	mSymbol._SetParentPtr(this);
}

//============================================================================================================

void UIList::_SetRootPtr (UIRoot* ptr)
{
	UIMenu::_SetRootPtr(ptr);
	mSymbol._SetRootPtr(ptr);
}

//============================================================================================================

void UIList::OnLayerChanged()
{
	UIMenu::OnLayerChanged();
	mSymbol.SetLayer(mLayer, false);
}

//============================================================================================================
// Triggered once per frame
//============================================================================================================

bool UIList::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= UIMenu::OnUpdate(dimensionsChanged);

	if (dimensionsChanged)
	{
		const UIFace* face = mSymbol.GetFace();

		if (face != 0)
		{
			const Vector2i& size = face->GetSize();
			float ratio = (float)size.x / size.y;

			if (mLabel.GetAlignment() == UILabel::Alignment::Right)
			{
				mSymbol.GetRegion().SetRight(0.0f, ratio * mImage.GetSubRegion().GetHeight());
			}
			else
			{
				mSymbol.GetRegion().SetLeft(1.0f, -ratio * mImage.GetSubRegion().GetHeight());
			}
		}
	}
	dimensionsChanged |= mSymbol.Update(mImage.GetSubRegion(), dimensionsChanged);
	return dimensionsChanged;
}

//============================================================================================================
// Fills the rendering queue
//============================================================================================================

void UIList::OnFill (UIQueue* queue)
{
	UIMenu::OnFill(queue);
	mSymbol.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIList::CustomSerializeFrom (const TreeNode& root)
{
	if (UIMenu::CustomSerializeFrom(root))
	{
		if (root.mTag == "Text")
		{
			root.mValue >> mSelection;
			if (mOnValueChange) mOnValueChange(this);
		}
		else if (root.mTag == "Skin")
		{
			mSymbol.SetSkin(mImage.GetSkin(), false);
			mSymbol.SetFace("Down Arrow", false);
		}
		return true;
	}
	return false;
}