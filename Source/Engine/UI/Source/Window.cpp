#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Window::Window() : mTitleHeight(0), mMovement(Movement::None), mResizable(true)
{
	mTitle.SetAlignment(Label::Alignment::Center);
	mTitlebar.GetRegion().SetBottom(0, 0);
	mTitle.SetLayer(1, false);
}

//============================================================================================================
// Changes the active skin
//============================================================================================================

void Window::SetSkin (const Skin* skin)
{
	mBackground.Set(skin, "Window: Background");
	mTitlebar.Set(skin, "Window: Titlebar");
}

//============================================================================================================
// Changes the height of the titlebar
//============================================================================================================

void Window::SetTitlebarHeight (byte val)
{
	if (mTitleHeight != val)
	{
		mTitleHeight = val;
		mTitlebar.GetRegion().SetBottom(0, mTitleHeight);
		mTitlebar.SetDirty();
	}
}

//============================================================================================================
// Resize the window so that the content region matches these dimensions
//============================================================================================================

Vector2f Window::GetSizeForContent (float x, float y)
{
	float paddingX = mRegion.GetWidth()  - mContent.GetWidth();
	float paddingY = mRegion.GetHeight() - mContent.GetHeight();
	return Vector2f(x + paddingX, y + paddingY);
}

//============================================================================================================
// Changes the parent pointer -- must be passed down to internal members
//============================================================================================================

void Window::_SetParentPtr (Area* ptr)
{
	Area::_SetParentPtr(ptr);
	mBackground._SetParentPtr(this);
	mTitlebar._SetParentPtr(this);
	mTitle._SetParentPtr(this);
}

//============================================================================================================
// Changes the root pointer -- must be passed down to internal members
//============================================================================================================

void Window::_SetRootPtr (Root* ptr)
{
	Area::_SetRootPtr(ptr);
	mBackground._SetRootPtr(ptr);
	mTitlebar._SetRootPtr(ptr);
	mTitle._SetRootPtr(ptr);
}

//============================================================================================================
// Marks this specific area as needing to be rebuilt
//============================================================================================================

void Window::SetDirty()
{
	mBackground.SetDirty();
	mTitlebar.SetDirty();
	mTitle.SetDirty();
}

//============================================================================================================
// Called when something changes in the texture
//============================================================================================================

void Window::OnTextureChanged (const ITexture* ptr)
{
	mBackground.OnTextureChanged(ptr);
	mTitlebar.OnTextureChanged(ptr);
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool Window::OnUpdate (bool dimensionsChanged)
{
	// Update the background. If background changes, it affects everything else
	dimensionsChanged |= mBackground.Update(mRegion, dimensionsChanged);

	// Update the titlebar and label
	mTitle.Update(mTitlebar.GetSubRegion(), mTitlebar.Update(mRegion, dimensionsChanged));

	// Update the content pane
	if (dimensionsChanged)
	{
		const Face* face = mBackground.GetFace();

		float border = (face) ? (float)face->GetBorder() : 0.0f;
		if (border < 0.0f) border = 0.0f;

		float height = mTitlebar.GetRegion().GetHeight();
		if (height < border) height = border;

		mContent.SetTop	(0.0f,  height);
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

void Window::OnFill (Queue* queue)
{
	mBackground.OnFill(queue);
	if (mTitlebar.GetRegion().IsVisible()) mTitlebar.OnFill(queue);
	if (mTitle.GetRegion().IsVisible())	mTitle.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Window::CustomSerializeFrom (const TreeNode& root)
{
	const Variable& value = root.mValue;

	if (root.mTag == "Skin")
	{
		SetSkin( mRoot->GetSkin(value.IsString() ? value.AsString() : value.GetString()) );
		return true;
	}
	else if (root.mTag == "Titlebar Height")
	{
		uint height;
		if (value >> height) SetTitlebarHeight((byte)height);
		return true;
	}
	else if (root.mTag == "Resizable")
	{
		bool val;
		if (value >> val) SetResizable(val);
		return true;
	}
	return mTitle.CustomSerializeFrom(root);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Window::CustomSerializeTo (TreeNode& root) const
{
	const Skin* skin = GetSkin();
	if (skin != 0) root.AddChild("Skin", skin->GetName());
	else root.AddChild("Skin");
	root.AddChild("Titlebar Height", mTitleHeight);
	root.AddChild("Resizable", mResizable);
	mTitle.CustomSerializeTo(root);
}

//============================================================================================================
// Windows can be moved and resized
//============================================================================================================

bool Window::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mMovement == Movement::Move)
	{
		mRegion.Adjust(delta.x, delta.y, delta.x, delta.y);
	}
	else if (mMovement != Movement::None)
	{
		Vector2f move;
		Vector2f min (100.0f, mTitlebar.GetRegion().GetHeight());
		if (min.y < 10.0f) min.y = 10.0f;

		if (mMovement & Movement::ResizeRight)
			move.x = (mRegion.GetWidth()  + delta.x < min.x) ? min.x - mRegion.GetWidth()  : delta.x;

		if (mMovement & Movement::ResizeBottom)
			move.y = (mRegion.GetHeight() + delta.y < min.y) ? min.y - mRegion.GetHeight() : delta.y;

		if (move.Dot() > 0.0f) mRegion.Adjust(0, 0, move.x, move.y);
	}
	if (mMovement == Movement::None) Area::OnMouseMove(pos, delta);
	return true;
}

//============================================================================================================
// Depending on where in the window the click happens, the window may be moved or resized
//============================================================================================================

bool Window::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft)
	{
		if (isDown)
		{
			const Face* face = mBackground.GetFace();
			float border = (face) ? (float)face->GetBorder() : 0.0f;
			if (border < 8) border = 8;

			if (mResizable)
			{
				// Resizing the window right border
				if (pos.x >= mRegion.GetRight() - border)
					mMovement |= Movement::ResizeRight;

				// Resizing the window by dragging the bottom border
				if (pos.y >= mRegion.GetBottom() - border)
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
	return Area::OnKey(pos, key, isDown) || (key > Key::MouseFirst && key < Key::MouseLast);
}