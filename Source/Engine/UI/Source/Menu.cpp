#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Clear all entries
//============================================================================================================

void UIMenu::ClearAllEntries()
{
	mEntries.Lock();
	mEntries.Clear();
	mEntries.Unlock();
}

//============================================================================================================
// Add an entry to the list
//============================================================================================================

void UIMenu::AddEntry (const String& entry)
{
	mEntries.Lock();
	{
		for (uint i = 0; i < mEntries.GetSize(); ++i)
		{
			if (mEntries[i] == entry)
			{
				mEntries.Unlock();
				return;
			}
		}
		mEntries.Expand() = entry;
	}
	mEntries.Unlock();
}

//============================================================================================================
// Set the list of entries by specifying the whole array
//============================================================================================================

void UIMenu::SetEntries (const Entries& entries)
{
	mEntries.Lock();
	{
		mEntries.Clear();

		for (uint i = 0; i < entries.GetSize(); ++i)
		{
			mEntries.Expand() = entries[i];
		}
	}
	mEntries.Unlock();
}

//============================================================================================================
// Shows the popup menu
//============================================================================================================

UIContext* UIMenu::_ShowMenu()
{
	UIContext* menu = mRoot->GetContextMenu(true);

	if (menu != 0)
	{
		menu->SetOnFocus		( bind(&UIMenu::_OnContextFocus, this) );
		menu->SetOnValueChange	( bind(&UIMenu::_OnContextValue, this) );
		menu->ClearAllEntries();

		// Only show the menu if it's actually valid
		if (mEntries.IsValid())
		{
			mEntries.Lock();
			{
				for (uint i = 0; i < mEntries.GetSize(); ++i)
					menu->AddEntry( mEntries[i] );
			}
			mEntries.Unlock();

			menu->SetSkin( GetSkin() );
			menu->SetFace( (mMenuFace.IsValid() ? mMenuFace : "Button: Pressed") );
			menu->SetFont( GetFont() );
			menu->SetColor( GetColor() );
			menu->SetAnchor( Vector3f(
				mRegion.GetCalculatedLeft(),
				mRegion.GetCalculatedBottom(),
				mRegion.GetCalculatedHeight()) );
			menu->SetMinWidth( mRegion.GetCalculatedWidth() );
			menu->SetAlignment( _GetMenuItemAlignment() );
			menu->Show();
		}

		// Regardless of whether the menu was shown or not, give it focus so it closes properly
		menu->SetFocus(true);
	}
	return menu;
}

//============================================================================================================
// Hides the popup menu
//============================================================================================================

UIContext* UIMenu::_HideMenu()
{
	UIContext* menu = mRoot->GetContextMenu();

	if (menu != 0)
	{
		menu->SetOnFocus(0);
		menu->SetOnValueChange(0);
		menu->Hide();
	}
	return menu;
}

//============================================================================================================
// Delegate functions triggered by the context menu
//============================================================================================================

bool UIMenu::_OnContextFocus (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		const Vector2i& pos = mRoot->GetMousePos();

		if (!mRegion.Contains(pos))
		{
			SetState( State::Pressed, false );
			mIgnoreMouseKey = false;
		}
	}
	return true;
}

//============================================================================================================

bool UIMenu::_OnContextValue (UIArea* area)
{
	UIContext* context = _HideMenu();

	if (context != 0)
	{
		// Have context intercept all mouse events until something else is selected
		mRoot->_SetEventArea(context);

		// Save the previous selection
		String previous (mLabel.GetText());

		// Set the current selection
		mLabel.SetText(context->GetText());

		// Inform the listener
		if (mOnValueChange)
		{
			// If the callback returns 'false', change the selection back to the previous value
			if (!mOnValueChange(this)) mLabel.SetText(previous);
		}

		// Turn off the button's "pressed" state
		if (!mSticky) SetState( State::Pressed, false );
	}
	return true;
}

//============================================================================================================
// Changes the state of the button
//============================================================================================================

bool UIMenu::SetState (uint state, bool val)
{
	bool wasPressed = ((GetState() & State::Pressed) != 0);
	bool retVal		= BaseClass::SetState(state, val);
	bool isPressed  = ((GetState() & State::Pressed) != 0);

	if ((GetState() & State::Enabled) != 0)
	{
		if (!wasPressed && isPressed)
		{
			_ShowMenu();
		}
		else if (wasPressed && !isPressed)
		{
			_HideMenu();
		}
	}
	return retVal;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIMenu::OnSerializeFrom (const TreeNode& node)
{
	if (BaseClass::OnSerializeFrom (node))
	{
		return true;
	}
	else if (node.mTag == "Menu")
	{
		mMenuFace = node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString();
		return true;
	}
	else if (node.mTag == "Entries")
	{
		if (node.mValue.IsStringArray())
		{
			mEntries = node.mValue.AsStringArray();
		}
		else
		{
			ClearAllEntries();
			for (uint i = 0; i < node.mChildren.GetSize(); ++i)
				AddEntry(node.mChildren[i].mTag);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIMenu::OnSerializeTo (TreeNode& node) const
{
	BaseClass::OnSerializeTo (node);
	node.AddChild("Menu", mMenuFace);
	node.AddChild("Entries", mEntries);
}