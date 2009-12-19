#include "../Include/_All.h"
using namespace R5;

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
// Notification of texture being changed
//============================================================================================================

void UIList::OnTextureChanged (const ITexture* ptr)
{
	mSymbol.OnTextureChanged(ptr);
	UIMenu::OnTextureChanged(ptr);
}

//============================================================================================================
// Triggered once per frame
//============================================================================================================

bool UIList::OnUpdate (bool dimensionsChanged)
{
	// Update the parent class
	dimensionsChanged |= UIMenu::OnUpdate(dimensionsChanged);

	const UIFace* face = mSymbol.GetFace();

	// If we haven't set the drop-down list arrow to be the symbol, let's do that now
	if (face == 0)
	{
		mSymbol.SetFace("Down Arrow");
		face = mSymbol.GetFace();
	}

	// If dimensions have changed, we should reposition the symbol picture
	if (dimensionsChanged)
	{
		if (face != 0)
		{
			const Vector2i& size = face->GetSize();
			float ratio = (float)size.x / size.y;
			float height = mImage.GetSubRegion().GetCalculatedHeight();
			float hmm = mImage.GetRegion().GetCalculatedHeight();

			if (mLabel.GetAlignment() == UILabel::Alignment::Right)
			{
				mSymbol.GetRegion().SetRight(0.0f, ratio * height);
			}
			else
			{
				mSymbol.GetRegion().SetLeft(1.0f, -ratio * height);
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

bool UIList::OnSerializeFrom (const TreeNode& node)
{
	if (UIMenu::OnSerializeFrom (node))
	{
		if (node.mTag == "Text")
		{
			String text;

			if (node.mValue >> text)
			{
				SetText(text);
				if (mOnValueChange) mOnValueChange(this);
			}
		}
		else if (node.mTag == "Skin")
		{
			mSymbol.SetSkin(mImage.GetSkin(), false);
		}
		return true;
	}
	return false;
}