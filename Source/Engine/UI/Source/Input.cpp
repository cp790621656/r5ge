#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIInput::UIInput() : mMaxHistorySize(0), mShowHistory(true)
{
	mLabel.SetLayer(1, false);
	mLabel.SetOnKey			( bind(&UIInput::_OnLabelKey,	this) );
	mLabel.SetOnFocus		( bind(&UIInput::_OnLabelFocus,	this) );
	mLabel.SetOnValueChange	( bind(&UIInput::_OnLabelValue,	this) );
}

//============================================================================================================
// Sets the maximum number of lines in the history list
//============================================================================================================

void UIInput::SetMaxHistorySize (uint	lines)
{
	if (lines < mMaxHistorySize)
	{
		mMaxHistorySize = lines;
		ClearHistory();
	}
	else
	{
		mMaxHistorySize = lines;
	}
}

//============================================================================================================
// Adds a new entry to the history list
//============================================================================================================

void UIInput::AddToHistory (const String& text)
{
	if (mMaxHistorySize > 0)
	{
		mHistory.Lock();
		{
			// If we haven't reached the limit yet, simply add this entry to the list
			if (mHistory.GetSize() < mMaxHistorySize)
			{
				mHistory.AddUnique(text);
			}
			else if ( !mHistory.Contains(text) )
			{
				// We've reached the limit -- shift all entries by one, eliminating the one added first
				for (uint i = 0; i < mHistory.GetSize() - 1; ++i)
				{
					mHistory[i] = mHistory[i+1];
				}

				// Add this entry to the very end
				mHistory[mHistory.GetSize() - 1] = text;
			}
		}
		mHistory.Unlock();
	}
}

//============================================================================================================
// If any key is pressed on the label it should hide history
//============================================================================================================

bool UIInput::_OnLabelKey (UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown && mLabel.GetRegion().Contains(pos))
	{
		if ( key > Key::MouseFirst && key < Key::MouseLast )
		{
			const UIContext* context = mRoot->GetContextMenu();

			if (context == 0 || context->GetCalculatedAlpha() == 0.0f)
			{
				_ShowHistory();
			}
			else
			{
				_HideHistory();
			}
		}
		else
		{
			_HideHistory();
		}
	}
	return true;
}

//============================================================================================================
// Private callback triggered when the label gains or loses focus
//============================================================================================================

bool UIInput::_OnLabelFocus (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		const UIArea*		focus	= mRoot->GetFocusArea();
		const UIContext*	context = mRoot->GetContextMenu();

		if (focus == 0 || !focus->IsChildOf(context))
		{
			_HideHistory();
		}
	}
	return true;
}

//============================================================================================================
// Private callback triggered when the label's value changes
//============================================================================================================

bool UIInput::_OnLabelValue (UIArea* area)
{
	if (mOnValueChange) mOnValueChange(this);
	return true;
}

//============================================================================================================
// Triggered when the context menu was clicked on, choosing a value
//============================================================================================================

bool UIInput::_OnContextValue	(UIArea* area)
{
	UIContext* menu = _HideHistory();

	if (menu != 0)
	{
		const String& selection = menu->GetLastSelection();
		SetText(selection);

		mShowHistory = false;
		mLabel.SetFocus(true);
		mShowHistory = true;
	}
	return true;
}

//============================================================================================================
// Shows the selection menu
//============================================================================================================

UIContext* UIInput::_ShowHistory()
{
	UIContext* menu = mRoot->GetContextMenu();

	if (mShowHistory && mMaxHistorySize > 0 && mHistory.GetSize() > 0 && menu != 0)
	{
		menu->SetOnFocus(0);
		menu->SetOnValueChange( bind(&UIInput::_OnContextValue, this) );
		menu->ClearAllEntries();

		mHistory.Lock();
		{
			uint max = (mHistory.GetSize() > mMaxHistorySize ? mHistory.GetSize() - mMaxHistorySize : 0);
			for (uint i = mHistory.GetSize(); i > max; )
				menu->AddEntry( mHistory[--i] );
		}
		mHistory.Unlock();

		const UIFace* face = GetFace();

		if (face != 0)
		{
			menu->SetSkin( GetSkin() );
			menu->SetFace( face->GetName() );
		}
		menu->SetFont( GetFont() );
		menu->SetColor( GetColor() );
		menu->SetAnchor( Vector3f(mRegion.GetCalculatedLeft(), mRegion.GetCalculatedBottom(), mRegion.GetCalculatedHeight()) );
		menu->SetMinWidth( mRegion.GetCalculatedWidth() );
		menu->SetAlignment( UILabel::Alignment::Left );
		menu->Show();
		menu->BringToFront();
	}
	return menu;
}

//============================================================================================================
// Hides the selection menu
//============================================================================================================

UIContext* UIInput::_HideHistory()
{
	UIContext* menu = mRoot->GetContextMenu();

	if (menu != 0 && menu->GetCalculatedAlpha() > 0.0f)
	{
		mShowHistory = true;

		menu->SetOnFocus(0);
		menu->SetOnValueChange(0);
		menu->Hide();
	}
	return menu;
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void UIInput::_SetParentPtr (UIArea* ptr)
{
	UIArea::_SetParentPtr(ptr);
	mImage._SetParentPtr(this);
	mLabel._SetParentPtr(this);
}

//============================================================================================================

void UIInput::_SetRootPtr (UIRoot* ptr)
{
	UIArea::_SetRootPtr(ptr);
	mImage._SetRootPtr(ptr);
	mLabel._SetRootPtr(ptr);
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIInput::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= mImage.Update(mRegion, dimensionsChanged);
	dimensionsChanged |= mLabel.Update(mImage.GetSubRegion(), dimensionsChanged);
	return dimensionsChanged;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIInput::OnSerializeFrom (const TreeNode& node)
{
	if ( mImage.OnSerializeFrom (node) ) return true;
	if ( mLabel.OnSerializeFrom (node) ) return true;

	if (node.mTag == "History Size")
	{
		node.mValue >> mMaxHistorySize;
		return true;
	}
	else if (node.mTag == "History")
	{
		for (uint i = 0; i < node.mChildren.GetSize(); ++i)
		{
			AddToHistory(node.mChildren[i].mTag);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIInput::OnSerializeTo (TreeNode& node) const
{
	mImage.OnSerializeTo (node);
	mLabel.OnSerializeTo (node);

	node.AddChild("History Size", mMaxHistorySize);

	if (mMaxHistorySize > 0 && mHistory.IsValid())
	{
		mHistory.Lock();
		{
			TreeNode& child = node.AddChild("History");

			uint max = (mHistory.GetSize() > mMaxHistorySize ? mHistory.GetSize() - mMaxHistorySize : 0);

			for (uint i = mHistory.GetSize(); i > max; )
				child.AddChild(mHistory[--i]);
		}
		mHistory.Unlock();
	}
}