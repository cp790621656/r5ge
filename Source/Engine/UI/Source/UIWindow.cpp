#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIWindow::UIWindow() : mPrefix(ClassID()), mTitleHeight(20), mMovement(Movement::None), mResizable(true)
{
	mBackground._SetParentPtr(this);
	mTitlebar._SetParentPtr(this);
	mTitle._SetParentPtr(this);
	mTitle.SetShadowColor(Color4ub(0, 0, 0, 255));
	mTitle.SetAlignment(UILabel::Alignment::Center);
	mTitlebar.GetRegion().SetBottom(0, mTitleHeight);
	mTitle.SetLayer(1, false);
}

//============================================================================================================
// Changes the active skin
//============================================================================================================

void UIWindow::SetSkin (const UISkin* skin, bool setDirty)
{
	mBackground.Set(skin, mPrefix + ": Background", setDirty);
	mTitlebar.Set(skin, mPrefix + ": Titlebar", setDirty);
}

//============================================================================================================
// Sets the prefix used by the window so that multiple windows can be created inside the same skin
//============================================================================================================

void UIWindow::SetPrefix (const String& prefix, bool setDirty)
{
	mPrefix = prefix;
	SetSkin(mBackground.GetSkin(), setDirty);
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
	if (mParent != 0) Update(mParent->GetRegion(), false, false);
	short paddingX = (short)(mRegion.GetCalculatedWidth()  - mContent.GetCalculatedWidth());
	short paddingY = (short)(mRegion.GetCalculatedHeight() - mContent.GetCalculatedHeight());
	return Vector2i(size.x + paddingX, size.y + paddingY);
}

//============================================================================================================
// Sets the UI root -- must be forwarded to the hidden children
//============================================================================================================

void UIWindow::_SetRootPtr (UIManager* ptr)
{
	UIWidget::_SetRootPtr(ptr);
	mBackground._SetRootPtr(ptr);
	mTitlebar._SetRootPtr(ptr);
	mTitle._SetRootPtr(ptr);
}

//============================================================================================================
// Clipping rectangle set before drawing the contents of the frame
//============================================================================================================

UIFrame::Rect UIWindow::GetClipRect() const
{
	int border = mBackground.GetFace() != 0 ? mBackground.GetFace()->GetBorder() : 0;
	if (border > 0) border = 0;

	Rect rect;
	rect.left	= Float::RoundToInt(mRegion.GetCalculatedLeft())	+ border;
	rect.top	= Float::RoundToInt(mRegion.GetCalculatedTop())		+ border;
	rect.right	= Float::RoundToInt(mRegion.GetCalculatedRight())	- border;
	rect.bottom	= Float::RoundToInt(mRegion.GetCalculatedBottom())	- border;
	return rect;
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
	// Use the default skin and font if none were specified
	if (GetSkin() == 0) SetSkin(mUI->GetDefaultSkin());
	if (GetFont() == 0) SetFont(mUI->GetDefaultFont());

	// Update the background. If background changes, it affects everything else
	dimensionsChanged |= mBackground.Update(mRegion, dimensionsChanged, true);

	// Update the title bar's background
	bool titleChanged = mTitlebar.Update(mRegion, dimensionsChanged, true);

	// Update the title bar's label
	mTitle.Update(mTitlebar.GetSubRegion(), titleChanged, true);

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
	mContent.Update(mRegion, dimensionsChanged, true);
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
		SetSkin( mUI->GetSkin(value.AsString()) );
		return true;
	}
	else if (node.mTag == "Prefix")
	{
		SetPrefix( node.mValue.AsString() );
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
	else if (node.mTag == "Back Color")
	{
		Color4ub c;
		if (value >> c) SetBackColor(c);
		return true;
	}
	return mTitle.OnSerializeFrom(node);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIWindow::OnSerializeTo (TreeNode& node) const
{
	const UISkin* skin = GetSkin();
	if (skin != 0 && skin != mUI->GetDefaultSkin())
		node.AddChild("Skin", skin->GetName());

	if (mPrefix != ClassID()) node.AddChild("Prefix", mPrefix);
	node.AddChild("Titlebar Height", mTitleHeight);
	node.AddChild("Resizable", mResizable);
	node.AddChild("Back Color", mBackground.GetBackColor());

	mTitle.OnSerializeTo(node);
}

//============================================================================================================
// Windows can be moved and resized
//============================================================================================================

void UIWindow::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mMovement == Movement::Move)
	{
		Adjust(delta.x, delta.y, delta.x, delta.y);
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

		if (move.Dot() > 0.0f) Adjust(0, 0, move.x, move.y);
	}
	if (mMovement == Movement::None) UIWidget::OnMouseMove(pos, delta);
}

//============================================================================================================
// Depending on where in the window the click happens, the window may be moved or resized
//============================================================================================================

void UIWindow::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
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
	if (mMovement == Movement::None) UIWidget::OnKeyPress(pos, key, isDown);
}