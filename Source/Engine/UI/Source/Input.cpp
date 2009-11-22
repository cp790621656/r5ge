#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Input::Input() : mMaxHistorySize(0), mShowHistory(true)
{
	mLabel.SetLayer(1, false);
	mLabel.SetOnKey		( bind(&Input::_OnLabelKey,		this) );
	mLabel.SetOnFocus		( bind(&Input::_OnLabelFocus,	this) );
	mLabel.SetOnValueChange( bind(&Input::_OnLabelValue,	this) );
}

//============================================================================================================
// Sets the maximum number of lines in the history list
//============================================================================================================

void Input::SetMaxHistorySize (uint	lines)
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

void Input::AddToHistory (const String& text)
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

bool Input::_OnLabelKey (Area* area, const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown && mLabel.GetRegion().Contains(pos))
	{
		if ( key > Key::MouseFirst && key < Key::MouseLast )
		{
			const Context* context = mRoot->GetContextMenu();

			if (context == 0 || context->GetAlpha() == 0.0f)
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

bool Input::_OnLabelFocus (Area* area, bool hasFocus)
{
	if (!hasFocus)
	{
		const Area*		focus	= mRoot->GetFocusArea();
		const Context*	context = mRoot->GetContextMenu();

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

bool Input::_OnLabelValue (Area* area)
{
	if (mOnValueChange) mOnValueChange(this);
	return true;
}

//============================================================================================================
// Triggered when the context menu was clicked on, choosing a value
//============================================================================================================

bool Input::_OnContextValue	(Area* area)
{
	Context* menu = _HideHistory();

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

Context* Input::_ShowHistory()
{
	Context* menu = mRoot->GetContextMenu();

	if (mShowHistory && mMaxHistorySize > 0 && mHistory.GetSize() > 0 && menu != 0)
	{
		menu->SetOnFocus(0);
		menu->SetOnValueChange( bind(&Input::_OnContextValue, this) );
		menu->ClearAllEntries();

		mHistory.Lock();
		{
			uint max = (mHistory.GetSize() > mMaxHistorySize ? mHistory.GetSize() - mMaxHistorySize : 0);
			for (uint i = mHistory.GetSize(); i > max; )
				menu->AddEntry( mHistory[--i] );
		}
		mHistory.Unlock();

		const Face* face = GetFace();

		if (face != 0)
		{
			menu->SetSkin( GetSkin() );
			menu->SetFace( face->GetName() );
		}
		menu->SetFont( GetFont() );
		menu->SetColor( GetColor() );
		menu->SetAnchor( Vector3f(mRegion.GetLeft(), mRegion.GetBottom(), mRegion.GetHeight()) );
		menu->SetMinWidth( mRegion.GetWidth() );
		menu->SetAlignment( Label::Alignment::Left );
		menu->Show();
		menu->BringToFront();
	}
	return menu;
}

//============================================================================================================
// Hides the selection menu
//============================================================================================================

Context* Input::_HideHistory()
{
	Context* menu = mRoot->GetContextMenu();

	if (menu != 0 && menu->GetAlpha() > 0.0f)
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

void Input::_SetParentPtr (Area* ptr)
{
	Area::_SetParentPtr(ptr);
	mImage._SetParentPtr(this);
	mLabel._SetParentPtr(this);
}

//============================================================================================================

void Input::_SetRootPtr (Root* ptr)
{
	Area::_SetRootPtr(ptr);
	mImage._SetRootPtr(ptr);
	mLabel._SetRootPtr(ptr);
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool Input::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= mImage.Update(mRegion, dimensionsChanged);
	dimensionsChanged |= mLabel.Update(mImage.GetSubRegion(), dimensionsChanged);
	return dimensionsChanged;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Input::CustomSerializeFrom (const TreeNode& root)
{
	if ( mImage.CustomSerializeFrom(root) ) return true;
	if ( mLabel.CustomSerializeFrom(root) ) return true;

	if (root.mTag == "History Size")
	{
		root.mValue >> mMaxHistorySize;
		return true;
	}
	else if (root.mTag == "History")
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			AddToHistory(root.mChildren[i].mTag);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void Input::CustomSerializeTo (TreeNode& root) const
{
	mImage.CustomSerializeTo(root);
	mLabel.CustomSerializeTo(root);

	root.AddChild("History Size", mMaxHistorySize);

	if (mMaxHistorySize > 0 && mHistory.IsValid())
	{
		mHistory.Lock();
		{
			TreeNode& node = root.AddChild("History");

			uint max = (mHistory.GetSize() > mMaxHistorySize ? mHistory.GetSize() - mMaxHistorySize : 0);

			for (uint i = mHistory.GetSize(); i > max; )
				node.AddChild(mHistory[--i]);
		}
		mHistory.Unlock();
	}
}