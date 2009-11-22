#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// List selection should update the label
//============================================================================================================

void List::_OnValue (const String& val)
{
	mLabel.SetText(val);
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void List::_SetParentPtr (Area* ptr)
{
	Menu::_SetParentPtr(ptr);
	mSymbol._SetParentPtr(this);
}

//============================================================================================================

void List::_SetRootPtr (Root* ptr)
{
	Menu::_SetRootPtr(ptr);
	mSymbol._SetRootPtr(ptr);
}

//============================================================================================================

void List::OnLayerChanged()
{
	Menu::OnLayerChanged();
	mSymbol.SetLayer(mLayer, false);
}

//============================================================================================================
// Triggered once per frame
//============================================================================================================

bool List::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= Menu::OnUpdate(dimensionsChanged);

	if (dimensionsChanged)
	{
		const Face* face = mSymbol.GetFace();

		if (face != 0)
		{
			const Vector2i& size = face->GetSize();
			float ratio = (float)size.x / size.y;

			if (mLabel.GetAlignment() == Label::Alignment::Right)
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

void List::OnFill (Queue* queue)
{
	Menu::OnFill(queue);
	mSymbol.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool List::CustomSerializeFrom (const TreeNode& root)
{
	if (Menu::CustomSerializeFrom(root))
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