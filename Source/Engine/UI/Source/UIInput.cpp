#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIInput::UIInput() : mMaxHistorySize(0), mShowHistory(true), mPadding(0)
{
	mLabel.SetLayer(1, false);
	mEventHandling = EventHandling::Full;

	USEventListener* listener = mLabel.AddScript<USEventListener>();
	ASSERT(listener != 0, "Missing listener?");

	listener->SetOnKey			( bind(&UIInput::_OnLabelKey,	this) );
	listener->SetOnFocus		( bind(&UIInput::_OnLabelFocus,	this) );
	listener->SetOnValueChange	( bind(&UIInput::_OnLabelValue,	this) );
}


//============================================================================================================
// Changes the padding on the sides of the internal text label
//============================================================================================================

void UIInput::SetTextPadding (int padding)
{
	if (mPadding != padding)
	{
		mPadding = padding;

		UIRegion& rgn = mLabel.GetRegion();
		rgn.SetLeft	 (0.0f,  (float)padding);
		rgn.SetRight (1.0f, -(float)padding);
		rgn.SetTop	 (0.0f,  (float)padding);
		rgn.SetBottom(1.0f, -(float)padding);
	}
}

//============================================================================================================
// Sets the maximum number of lines in the history list
//============================================================================================================

void UIInput::SetMaxHistorySize (uint lines)
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
}

//============================================================================================================
// If any key is pressed on the label it should hide history
//============================================================================================================

void UIInput::_OnLabelKey (UIWidget* widget, const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown && mLabel.GetRegion().Contains(pos))
	{
		if ( key > Key::MouseFirst && key < Key::MouseLast )
		{
			const UIContext* context = mUI->GetContextMenu();

			if (mLabel.GetSelectedText().IsEmpty() &&
				(context == 0 || context->GetRegion().GetCalculatedAlpha() == 0.0f))
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
}

//============================================================================================================
// Private callback triggered when the label gains or loses focus
//============================================================================================================

void UIInput::_OnLabelFocus (UIWidget* widget, bool hasFocus)
{
	if (!hasFocus)
	{
		const UIWidget*		focus	= mUI->GetFocusArea();
		const UIContext*	context = mUI->GetContextMenu();

		if (focus == 0 || !focus->IsChildOf(context))
		{
			_HideHistory();
		}
	}
}

//============================================================================================================
// Triggered when the context menu was clicked on, choosing a value
//============================================================================================================

void UIInput::_OnContextValue (UIWidget* widget)
{
	UIContext* menu = _HideHistory();

	if (menu != 0)
	{
		const String& selection = menu->GetText();
		SetText(selection);

		mShowHistory = false;
		mLabel.SetFocus(true);
		mShowHistory = true;
	}
}

//============================================================================================================
// Shows the selection menu
//============================================================================================================

UIContext* UIInput::_ShowHistory()
{
	UIContext* menu = mUI->GetContextMenu(true);

	if (mShowHistory && mMaxHistorySize > 0 && mHistory.GetSize() > 0 && menu != 0)
	{
		USEventListener* listener = menu->AddScript<USEventListener>();
		listener->SetOnFocus(0);
		listener->SetOnValueChange( bind(&UIInput::_OnContextValue, this) );

		menu->ClearAllEntries();

		uint max = (mHistory.GetSize() > mMaxHistorySize ? mHistory.GetSize() - mMaxHistorySize : 0);
		for (uint i = mHistory.GetSize(); i > max; )
			menu->AddEntry( mHistory[--i] );

		const UIFace* face = GetFace();

		if (face != 0)
		{
			menu->SetSkin( GetSkin() );
			menu->SetFace( face->GetName() );
		}
		menu->SetFont( GetFont() );
		menu->SetTextColor( GetTextColor() );
		menu->SetBackColor( GetBackColor() );
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
	UIContext* menu = mUI->GetContextMenu();

	if (menu != 0 && menu->GetAlpha() > 0.0f)
	{
		mShowHistory = true;
		USEventListener* listener = menu->GetScript<USEventListener>();

		if (listener != 0)
		{
			listener->SetOnFocus(0);
			listener->SetOnValueChange(0);
		}
		menu->Hide();
	}
	return menu;
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void UIInput::_SetParentPtr (UIWidget* ptr)
{
	UIWidget::_SetParentPtr(ptr);
	mImage._SetParentPtr(this);
	mLabel._SetParentPtr(this);
}

//============================================================================================================

void UIInput::_SetRootPtr (UIManager* ptr)
{
	UIWidget::_SetRootPtr(ptr);
	mImage._SetRootPtr(ptr);
	mLabel._SetRootPtr(ptr);
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIInput::OnUpdate (bool dimensionsChanged)
{
	if (GetSkin() == 0 && mUI->GetDefaultSkin() != 0)
	{
		SetSkin(mUI->GetDefaultSkin());
		SetFace(String("%s: Background", GetClassID()));
	}

	dimensionsChanged |= mImage.Update(mRegion, dimensionsChanged, true);
	dimensionsChanged |= mLabel.Update(mImage.GetSubRegion(), dimensionsChanged, true);
	return dimensionsChanged;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIInput::OnSerializeFrom (const TreeNode& node)
{
	if ( mImage.OnSerializeFrom (node) ) return true;
	if ( mLabel.OnSerializeFrom (node) ) return true;

	if (node.mTag == "Text Padding")
	{
		int padding (0);
		if (node.mValue >> padding) SetTextPadding(padding);
		return true;
	}
	else if (node.mTag == "History Size")
	{
		node.mValue >> mMaxHistorySize;
		return true;
	}
	else if (node.mTag == "History")
	{
		if (node.mValue.IsStringArray())
		{
			mHistory = node.mValue.AsStringArray();
			if (mMaxHistorySize < mHistory.GetSize()) mMaxHistorySize = mHistory.GetSize();
		}
		else // Legacy format support, will be removed
		{
			for (uint i = 0; i < node.mChildren.GetSize(); ++i)
			{
				AddToHistory(node.mChildren[i].mTag);
			}
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

	if (mPadding != 0) node.AddChild("Text Padding", mPadding);

	mLabel.OnSerializeTo (node);

	node.AddChild("History Size", mMaxHistorySize);

	if (mMaxHistorySize > 0 && mHistory.IsValid())
	{
		node.AddChild("History", mHistory);
	}
}