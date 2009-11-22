#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Clear all entries
//============================================================================================================

void Menu::ClearAllEntries()
{
	mEntries.Lock();
	mEntries.Clear();
	mEntries.Unlock();
}

//============================================================================================================
// Add an entry to the list
//============================================================================================================

void Menu::AddEntry (const String& entry)
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

void Menu::SetEntries (const Entries& entries)
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

Context* Menu::_ShowMenu()
{
	Context* menu = mRoot->GetContextMenu(true);

	if (menu != 0)
	{
		menu->SetOnFocus		( bind(&Menu::_OnContextFocus, this) );
		menu->SetOnValueChange	( bind(&Menu::_OnContextValue, this) );
		menu->ClearAllEntries();

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
		menu->SetAnchor( Vector3f(mRegion.GetLeft(), mRegion.GetBottom(), mRegion.GetHeight()) );
		menu->SetMinWidth( mRegion.GetWidth() );
		menu->SetAlignment( _GetMenuItemAlignment() );
		menu->Show();
		menu->SetFocus(true);
	}
	return menu;
}

//============================================================================================================
// Hides the popup menu
//============================================================================================================

Context* Menu::_HideMenu()
{
	Context* menu = mRoot->GetContextMenu();

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

bool Menu::_OnContextFocus (Area* area, bool hasFocus)
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

bool Menu::_OnContextValue (Area* area)
{
	Context* context = _HideMenu();

	if (context != 0)
	{
		// Have context intercept all mouse events until something else is selected
		mRoot->_SetEventArea(context);

		// Save the selection
		mSelection = context->GetLastSelection();

		// Inform the derived class
		_OnValue(mSelection);

		// Inform the listener
		if (mOnValueChange) mOnValueChange(this);

		// Turn off the button's "pressed" state
		if (!mSticky) SetState( State::Pressed, false );
	}
	return true;
}

//============================================================================================================
// Changes the state of the button
//============================================================================================================

bool Menu::SetState (uint state, bool val)
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

bool Menu::CustomSerializeFrom (const TreeNode& root)
{
	if (BaseClass::CustomSerializeFrom(root))
	{
		return true;
	}
	else if (root.mTag == "Menu")
	{
		mMenuFace = root.mValue.IsString() ? root.mValue.AsString() : root.mValue.GetString();
		return true;
	}
	else if (root.mTag == "Entries")
	{
		if (root.mValue.IsStringArray())
		{
			mEntries = root.mValue.AsStringArray();
		}
		else
		{
			ClearAllEntries();
			for (uint i = 0; i < root.mChildren.GetSize(); ++i)
				AddEntry(root.mChildren[i].mTag);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Menu::CustomSerializeTo (TreeNode& root) const
{
	BaseClass::CustomSerializeTo(root);
	root.AddChild("Menu", mMenuFace);
	root.AddChild("Entries", mEntries);
}