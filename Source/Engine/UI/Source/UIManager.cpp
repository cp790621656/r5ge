#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Default function that fills the tooltip
//============================================================================================================

UIManager::UIManager() :
	mSerializable	(true),
	mDimsChanged	(false),
	mIsDirty		(false),
	mHover			(0),
	mFocus			(0),
	mSelected		(0),
	mContext		(0),
	mDefaultSkin	(0),
	mDefaultFont	(0),
	mTtArea			(0),
	mTtTime			(100000.0f),	// High number so tooltip doesn't show up right away
	mTtDelay		(1.0f),
	mTtQueued		(false),
	mTtShown		(false)
{
	memset(mKey, 0, 256);

	mRoot._SetRootPtr(this);
	mRoot.SetName("UI Root");

	mTooltip._SetRootPtr(this);
	mTooltip.GetRegion().SetAlpha(0.0f);
	mTooltip.SetName("Tooltip");
	mTooltip.SetReceivesEvents(false);

	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;

		UIWidget::Register<UIWidget>();
		UIWidget::Register<UIHighlight>();
		UIWidget::Register<UIPicture>();
		UIWidget::Register<UISubPicture>();
		UIWidget::Register<UILabel>();
		UIWidget::Register<UITextArea>();
		UIWidget::Register<UIInput>();
		UIWidget::Register<UIContext>();
		UIWidget::Register<UIMenu>();
		UIWidget::Register<UIList>();
		UIWidget::Register<UIWindow>();
		UIWidget::Register<UIAnimatedFrame>();
		UIWidget::Register<UIAnimatedSlider>();
		UIWidget::Register<UIAnimatedButton>();
		UIWidget::Register<UIAnimatedCheckbox>();
		UIWidget::Register<UIShadedArea>();
		UIScript::Register<USEventListener>();
	}
}

//============================================================================================================
// If an widget is being deleted, the root must be told so it removes all local references to it
//============================================================================================================

void UIManager::RemoveAllReferencesTo (const UIWidget* widget)
{
	if (mHover		== widget) mHover		= 0;
	if (mFocus		== widget) mFocus		= 0;
	if (mSelected	== widget) mSelected	= 0;
	if (mTtArea		== widget) mTtArea		= 0;
}

//============================================================================================================
// Retrieves the specified skin (creates if necessary)
//============================================================================================================

UISkin* UIManager::GetSkin (const String& name)
{
	typedef UISkin* SkinPtr;
	SkinPtr ptr (0);

	if (name.IsValid())
	{
		SkinPtr& skin = mSkins[name];
		if (skin == 0) skin = new UISkin(this, name);
		ptr = skin;
	}
	return ptr;
}

//============================================================================================================
// Retrieves a pointer to the context menu
//============================================================================================================

UIContext* UIManager::GetContextMenu (bool createIfMissing)
{
	if (mContext == 0 && createIfMissing)
	{
		mContext = AddWidget<UIContext>("_Context Menu_");
		mContext->SetSerializable(false);
	}
	return mContext;
}

//============================================================================================================
// Informs all areas that the texture has changed
//============================================================================================================

void UIManager::_TextureChanged (const ITexture* ptr)
{
	mRoot._TextureChanged(ptr);
	mTooltip._TextureChanged(ptr);
}

//============================================================================================================
// Changes the hovering widget
//============================================================================================================

void UIManager::_SetHoverArea (UIWidget* ptr)
{
	if (mHover != ptr)
	{
		// Update the hover widget
		if (mHover) mHover->OnMouseOver(false);
		mHover = ptr;
		if (mHover) mHover->OnMouseOver(true);
	}
}

//============================================================================================================
// Manual way of changing the focus widget
//============================================================================================================

void UIManager::_SetFocusArea (UIWidget* ptr)
{
	if (mFocus != ptr)
	{
		UIWidget* oldInput = mFocus;
		mFocus = ptr;
		
		if (oldInput) oldInput->OnFocus(false);
		if (mFocus) mFocus->OnFocus(true);
		if (mFocus) mFocus->BringToFront();
	}
}

//============================================================================================================
// Manual way of changing the selected widget
//============================================================================================================

void UIManager::_SetEventArea (UIWidget* ptr)
{
	mSelected = ptr;
}

//============================================================================================================
// Creates a default tooltip (returns whether the tooltip is valid)
//============================================================================================================

bool UIManager::CreateDefaultTooltip (UIWidget* widget)
{
	// If there is no widget to work with, or there is no default font, no need to do anything
	if (widget == 0 || mDefaultFont == 0) return false;

	// If the widget has no tooltip text, no need to do anything
	const String& text (widget->GetTooltip());
	if (text.IsEmpty()) return false;

	byte textSize  = mDefaultFont->GetSize();
	uint textWidth = mDefaultFont->GetLength(text, 0, 0xFFFFFFFF, IFont::Tags::Skip);

	// If the printable text has no width, no point in showing an empty tooltip
	if (textWidth == 0) return false;

	UIWidget* parent (0);

	if (mDefaultSkin == 0 || mDefaultSkin->GetFace("Tooltip")->GetSize() == 0)
	{
		// No skin available -- use a simple highlight
		UIHighlight* hl = mTooltip.AddWidget<UIHighlight>("Tooltip Backdrop");
		hl->SetColor( Color4f(0.0f, 0.0f, 0.0f, 1.0f) );

		UIRegion& hlrgn (hl->GetRegion());
		hlrgn.SetLeft	(0.0f, -3.0f);
		hlrgn.SetRight	(1.0f,  3.0f);
		hlrgn.SetTop	(0.0f, -3.0f);
		hlrgn.SetBottom	(1.0f,  3.0f);

		UIRegion& rgn = mTooltip.GetRegion();
		rgn.SetRight	(0.0f, (float)textWidth);
		rgn.SetBottom	(0.0f, (float)textSize);

		parent = hl;
	}
	else
	{
		UISubPicture* img = mTooltip.AddWidget<UISubPicture>("Tooltip Background");
		img->Set(mDefaultSkin, "Tooltip");

		short border = img->GetFace()->GetBorder();
		if (border < 0) border = 0;

		UIRegion& rgn = mTooltip.GetRegion();
		rgn.SetRight (0.0f, (float)(textWidth + border * 2));
		rgn.SetBottom(0.0f, (float)(textSize  + border * 2));

		parent = img;
	}

	if (parent != 0)
	{
		UILabel* lbl = parent->AddWidget<UILabel>("Tooltip Label");
		lbl->SetLayer(1, true);
		lbl->SetFont(mDefaultFont);
		lbl->SetText(text);
	}
	return true;
}

//============================================================================================================
// Aligns the tooltip using default logic (returns whether the tooltip is valid)
//============================================================================================================

bool UIManager::AlignDefaultTooltip()
{
	// Update
	UIRegion& rgn = mTooltip.GetRegion();
	float left    = (float)mMousePos.x;
	float top     = (float)mMousePos.y;
	float topOff  = top + 25.0f;
	float width   = rgn.GetRight().mAbsolute - rgn.GetLeft().mAbsolute;
	float half	  = width * 0.5f;
	float height  = rgn.GetBottom().mAbsolute - rgn.GetTop().mAbsolute;

	if (width > 0.0f && height > 0.0f)
	{
		// The tooltip should be centered horizontally
		if (left - half < 0) left = 0;
		else if (left + half > mSize.x)	left = mSize.x - width;
		else left = left - half;

		// Vertically it should be either some space below, or directly above the mouse
		top = (topOff + height < mSize.y) ? topOff : top - height - 5;

		// Set the tooltip's region
		rgn.SetRect(left, top, width, height);
		return true;
	}
	return false;
}

//============================================================================================================
// Retrieves an event listener script for the widget of specified name, creating a new one if necessary
//============================================================================================================

USEventListener* UIManager::_AddListener (const String& name)
{
	USEventListener* listener (0);
	Lock();
	{
		UIWidget* widget = mRoot._FindWidget(name);

		if (widget != 0)
		{
			listener = widget->AddScript<USEventListener>();
		}
		else
		{
			uint key = HashKey(name);
			listener = mListeners[key];

			if (listener == 0)
			{
				listener = new USEventListener();
				mListeners[key] = listener;
			}
		}
	}
	Unlock();
	return listener;
}

//============================================================================================================
// Retrieves a previously created USEventListener script, if any
//============================================================================================================

USEventListener* UIManager::_GetListener (const String& name)
{
	USEventListener* listener = mListeners.GetIfExists(name);

	if (listener != 0)
	{
		mListeners[name] = 0;
		return listener;
	}
	return 0;
}

//============================================================================================================
// Hides the tooltip if it's currently visible
//============================================================================================================

void UIManager::_HideTooltip()
{
	mTtQueued = false;
	mTtTime = GetCurrentTime();

	if (mTtShown)
	{
		mTtShown = false;
		mTooltip.Hide();
	}
}

//============================================================================================================
// Calls mOnFillTooltip function, and fills out the default tooltip if the return val is 'false'
//============================================================================================================

bool UIManager::_FillTooltip (UIWidget* widget)
{
	if (widget)
	{
		mTooltip.DestroyAllWidgets();
		UIRegion& rgn = mTooltip.GetRegion();
		rgn.SetRect(0, 0, 0, 0);

		return (mTtDelegate ? mTtDelegate(mTooltip, widget) :
			CreateDefaultTooltip(widget) && AlignDefaultTooltip());
	}
	return false;
}

//============================================================================================================
// Mark the entire Root as dirty so it's rebuilt on the next frame
//============================================================================================================

void UIManager::OnResize(const Vector2i& size)
{
	if (mSize != size)
	{
		Lock();
		{
			mSize = size;
			mDimsChanged = true;
		}
		Unlock();
	}
}

//============================================================================================================
// Run through all areas and ask them to update their regions, and associated widgets
//============================================================================================================

bool UIManager::Update()
{
	Lock();
	{
		// Update all widgets
		mRoot.Update(mSize, mDimsChanged);
		mDimsChanged = false;

		// Show the tooltip if the tooltip widget is valid, and the time has been reached
		if (mTtQueued && (mTtTime + mTtDelay < GetCurrentTime()))
		{
			mTtQueued = false;

			// Only actually show it if there is text to show
			if (_FillTooltip(mTtArea))
			{
				mTtShown = true;
				mTooltip.Show();
			}
		}

		// Update the tooltip
		mTooltip.Update(mSize, mDimsChanged);
	}
	Unlock();
	return mIsDirty;
}

//============================================================================================================
// Handle mouse movement
//============================================================================================================

bool UIManager::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	bool retVal (false);

	Lock();
	{
		_HideTooltip();
		mMousePos = pos;

		if (mSelected)
		{
			// If we have an widget that has focus (mouse key was held down on it), inform it of mouse movement
			retVal = mSelected->OnMouseMove(pos, delta);
		}
		else
		{
			// If some mouse key is held down, ignore mouse movement altogether
			for (uint i = Key::MouseFirst + 1; i < Key::MouseLast; ++i)
			{
				if (mKey[i])
				{
					Unlock();
					return false;
				}
			}

			// Hover over the widget
			_SetHoverArea( mRoot._FindWidget(pos) );

			// Queue the tooltip
			mTtArea	= mHover;
			mTtTime	= GetCurrentTime();
			mTtQueued = true;

			// Inform the hovering widget of mouse movement
			if (mHover != 0) retVal = mHover->OnMouseMove(pos, delta);
		}
	}
	Unlock();
	return retVal;
}

//============================================================================================================
// Handle keys
//============================================================================================================

bool UIManager::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	bool retVal (false);
	Lock();
	{
		_HideTooltip();
		mKey[key] = isDown;

		if (key > Key::MouseFirst && key < Key::MouseLast)
		{
			if (isDown)
			{
				mSelected = mRoot._FindWidget(pos);
				_SetFocusArea(mSelected);
			}
			else
			{
				mSelected = 0;
				_SetHoverArea( mRoot._FindWidget(pos) );
			}
		}

		// Inform the hovering widget of the key event
		if (mFocus != 0) retVal = mFocus->OnKeyPress(pos, key, isDown);
	}
	Unlock();
	return retVal;
}

//============================================================================================================
// Respond to the mouse wheel event
//============================================================================================================

bool UIManager::OnScroll (const Vector2i& pos, float delta)
{
	bool retVal (false);
	Lock();
	{
		_HideTooltip();
		if (mHover != 0) retVal = mHover->OnScroll(pos, delta);
	}
	Unlock();
	return retVal;
}

//============================================================================================================
// Respond to printable characters
//============================================================================================================

bool UIManager::OnChar (byte character)
{
	bool retVal (false);
	Lock();
	{
		_HideTooltip();

		if (mFocus != 0)
		{
			mFocus->OnChar(character);
			retVal = true;
		}
	}
	Unlock();
	return retVal;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIManager::SerializeFrom (const TreeNode& root)
{
	Lock();
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if (tag == "UI")
			{
				return SerializeFrom(node);
			}
			else if (tag == "Default Skin")
			{
				mDefaultSkin = GetSkin( value.IsString() ? value.AsString() : value.GetString() );
			}
			else if (tag == "Default Font")
			{
				mDefaultFont = GetFont( value.IsString() ? value.AsString() : value.GetString() );
			}
			else if (tag == "Tooltip Delay")
			{
				value >> mTtDelay;
			}
			else if (tag == "Skin")
			{
				UISkin* skin = GetSkin( value.IsString() ? value.AsString() : value.GetString() );
				skin->SerializeFrom(node);
			}
			else if (tag == "Layout")
			{
				mRoot.SerializeFrom(node);
			}
		}
	}
	Unlock();
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool UIManager::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		Lock();
		{
			TreeNode& node = root.AddChild("UI");
			TreeNode& skin = node.AddChild("Default Skin");
			TreeNode& font = node.AddChild("Default Font");

			if (mDefaultSkin != 0) skin.mValue = mDefaultSkin->GetName();
			if (mDefaultFont != 0) font.mValue = mDefaultFont->GetName();
			node.AddChild("Tooltip Delay", mTtDelay);

			// Serialize all skins
			const PointerArray<UISkin>& allSkins = mSkins.GetAllValues();

			if (allSkins.IsValid())
			{
				for (uint i = 0; i < allSkins.GetSize(); ++i)
				{
					if (allSkins[i] != 0)
					{
						allSkins[i]->SerializeTo(node);
					}
				}
			}

			// Serialize all children
			const UIWidget::Children& children = mRoot.GetChildren();

			if (children.IsValid())
			{
				TreeNode& layout = node.AddChild("Layout");

				for (uint i = 0; i < children.GetSize(); ++i)
				{
					children[i]->SerializeTo(layout);
				}
			}
		}
		Unlock();
		return true;
	}
	return false;
}

//============================================================================================================
// Draw everything
//============================================================================================================

uint UIManager::Draw()
{
	mIsDirty = false;
	uint triangles (0);

	if (mRoot.GetChildren().IsValid())
	{
		Lock();
		{
			OnPreDraw();
			{
				mRoot.Draw();					// Draw all children
				triangles += mTooltip.Draw();	// Draw the tooltip
			}
			OnPostDraw();
		}
		Unlock();
	}
	return triangles;
}