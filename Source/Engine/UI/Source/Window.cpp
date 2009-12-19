#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIWindow::UIWindow() : mTitleHeight(0), mMovement(Movement::None), mResizable(true)
{
	mBackground._SetParentPtr(this);
	mTitlebar._SetParentPtr(this);
	mTitle._SetParentPtr(this);
	mTitle.SetShadow(true);
	mTitle.SetAlignment(UILabel::Alignment::Center);
	mTitlebar.GetRegion().SetBottom(0, 0);
	mTitle.SetLayer(1, false);
}

//============================================================================================================
// Changes the active skin
//============================================================================================================

void UIWindow::SetSkin (const UISkin* skin, bool setDirty)
{
	mBackground.Set(skin, "Window: Background", setDirty);
	mTitlebar.Set(skin, "Window: Titlebar", setDirty);
}

//============================================================================================================
// Changes the height of the titlebar
//============================================================================================================

void UIWindow::SetTitlebarHeight (byte val)
{
	if (mTitleHeight != val)
	{
		mTitleHeight = val;
		mTitlebar.GetRegion().SetBottom(0, mTitleHeight);
		mTitlebar.SetDirty();
	}
}

//============================================================================================================
// Resizes the window to fit the specified size
//============================================================================================================

void UIWindow::ResizeToFit (const Vector2i& size)
{
	Vector2f mySize ( GetSizeForContent(size) );

	const UIAnchor& left = mRegion.GetLeft();
	const UIAnchor& top  = mRegion.GetTop();

	mRegion.SetRight ( left.mRelative, left.mAbsolute + mySize.x );
	mRegion.SetBottom( top.mRelative,  top.mAbsolute  + mySize.y );
}

//============================================================================================================
// Resize the window so that the content region matches these dimensions
//============================================================================================================

Vector2i UIWindow::GetSizeForContent (const Vector2i& size)
{
	if (mParent != 0) Update(mParent->GetRegion());
	short paddingX = (short)(mRegion.GetCalculatedWidth()  - mContent.GetCalculatedWidth());
	short paddingY = (short)(mRegion.GetCalculatedHeight() - mContent.GetCalculatedHeight());
	return Vector2i(size.x + paddingX, size.y + paddingY);
}

//============================================================================================================
// Changes the parent pointer -- must be passed down to internal members
//============================================================================================================

void UIWindow::_SetParentPtr (UIArea* ptr)
{
	UIArea::_SetParentPtr(ptr);
}

//============================================================================================================
// Changes the root pointer -- must be passed down to internal members
//============================================================================================================

void UIWindow::_SetRootPtr (UIRoot* ptr)
{
	UIArea::_SetRootPtr(ptr);
	mBackground._SetRootPtr(ptr);
	mTitlebar._SetRootPtr(ptr);
	mTitle._SetRootPtr(ptr);

	// Assume the default skin is used
	SetSkin(ptr->GetDefaultSkin(), false);
}

//============================================================================================================
// Marks this specific area as needing to be rebuilt
//============================================================================================================

void UIWindow::SetDirty()
{
	mBackground.SetDirty();
	mTitlebar.SetDirty();
	mTitle.SetDirty();
}

//============================================================================================================
// Called when something changes in the texture
//============================================================================================================

void UIWindow::OnTextureChanged (const ITexture* ptr)
{
	mBackground.OnTextureChanged(ptr);
	mTitlebar.OnTextureChanged(ptr);
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIWindow::OnUpdate (bool dimensionsChanged)
{
	// Update the background. If background changes, it affects everything else
	dimensionsChanged |= mBackground.Update(mRegion, dimensionsChanged);

	// Update the title bar's background
	bool titleChanged = mTitlebar.Update(mRegion, dimensionsChanged);

	// Update the title bar's label
	mTitle.Update(mTitlebar.GetSubRegion(), titleChanged);

	// Update the content pane
	if (dimensionsChanged)
	{
		const UIFace* face = mBackground.GetFace();

		float border = (face) ? (float)face->GetBorder() : 0.0f;
		if (border < 0.0f) border = 0.0f;

		float height = mTitlebar.GetRegion().GetCalculatedHeight();
		if (height < border) height = border;

		mContent.SetTop		(0.0f,  height);
		mContent.SetBottom	(1.0f, -border);
		mContent.SetLeft	(0.0f,  border);
		mContent.SetRight	(1.0f, -border);
	}

	// Update the content region
	mContent.Update(mRegion, dimensionsChanged);
	return dimensionsChanged;
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIWindow::OnFill (UIQueue* queue)
{
	mBackground.OnFill(queue);
	if (mTitlebar.GetRegion().IsVisible()) mTitlebar.OnFill(queue);
	if (mTitle.GetRegion().IsVisible())	mTitle.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIWindow::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;

	if (node.mTag == "Skin")
	{
		SetSkin( mRoot->GetSkin(value.IsString() ? value.AsString() : value.GetString()) );
		return true;
	}
	else if (node.mTag == "Titlebar Height")
	{
		uint height;
		if (value >> height) SetTitlebarHeight((byte)height);
		return true;
	}
	else if (node.mTag == "Resizable")
	{
		bool val;
		if (value >> val) SetResizable(val);
		return true;
	}
	return mTitle.OnSerializeFrom (node);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIWindow::OnSerializeTo (TreeNode& node) const
{
	const UISkin* skin = GetSkin();
	if (skin != 0 && skin != mRoot->GetDefaultSkin())
		node.AddChild("Skin", skin->GetName());

	node.AddChild("Titlebar Height", mTitleHeight);
	node.AddChild("Resizable", mResizable);

	mTitle.OnSerializeTo(node);
}

//============================================================================================================
// Windows can be moved and resized
//============================================================================================================

bool UIWindow::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mMovement == Movement::Move)
	{
		mRegion.Adjust(delta.x, delta.y, delta.x, delta.y);
	}
	else if (mMovement != Movement::None)
	{
		Vector2f move;
		Vector2f min (100.0f, mTitlebar.GetRegion().GetCalculatedHeight());
		if (min.y < 10.0f) min.y = 10.0f;

		if (mMovement & Movement::ResizeRight)
			move.x = (mRegion.GetCalculatedWidth()  + delta.x < min.x) ? min.x - mRegion.GetCalculatedWidth()  : delta.x;

		if (mMovement & Movement::ResizeBottom)
			move.y = (mRegion.GetCalculatedHeight() + delta.y < min.y) ? min.y - mRegion.GetCalculatedHeight() : delta.y;

		if (move.Dot() > 0.0f) mRegion.Adjust(0, 0, move.x, move.y);
	}
	if (mMovement == Movement::None) UIArea::OnMouseMove(pos, delta);
	return true;
}

//============================================================================================================
// Depending on where in the window the click happens, the window may be moved or resized
//============================================================================================================

bool UIWindow::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft)
	{
		if (isDown)
		{
			const UIFace* face = mBackground.GetFace();
			float border = (face) ? (float)face->GetBorder() : 0.0f;
			if (border < 8) border = 8;

			if (mResizable)
			{
				// Resizing the window right border
				if (pos.x >= mRegion.GetCalculatedRight() - border)
					mMovement |= Movement::ResizeRight;

				// Resizing the window by dragging the bottom border
				if (pos.y >= mRegion.GetCalculatedBottom() - border)
					mMovement |= Movement::ResizeBottom;
			}

			// Moving the entire window by dragging the titlebar
			if (mMovement == Movement::None && (mTitleHeight == 0 || mTitlebar.GetRegion().Contains(pos)))
				mMovement = Movement::Move;
		}
		// Cancel all movement
		else mMovement = Movement::None;
	}

	// If we're moving the window, don't inform the children of this key event
	if (mMovement != Movement::None) return true;

	// Inform the children and intercept all mouse events
	return UIArea::OnKey(pos, key, isDown) || (key > Key::MouseFirst && key < Key::MouseLast);
}