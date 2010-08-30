#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Helper function that counts the number of lines if printed from the start
//============================================================================================================

uint CountLines (const TreeNode& root)
{
	uint count (1);

	if (root.mFlags.Get(1 << 31))
	{
		FOREACH(i, root.mChildren) count += CountLines(root.mChildren[i]);
	}
	return count;
}

//============================================================================================================
// Constructor should set all default parameters to be visible
//============================================================================================================

UITreeView::UITreeView() :
	mFont		(0),
	mPadding	(1),
	mStart		(0),
	mTextColor	(255, 255, 255, 255),
	mShadowColor(0),
	mClickStamp	(0),
	mSelection	(0),
	mPrefix		(ClassID())
{
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void UITreeView::_SetParentPtr (UIWidget* ptr)
{
	UIWidget::_SetParentPtr(ptr);
	mSelectPic._SetParentPtr(this);
	mPlusPic._SetParentPtr(this);
	mMinusPic._SetParentPtr(this);
}

//============================================================================================================

void UITreeView::_SetRootPtr (UIManager* ptr)
{
	UIWidget::_SetRootPtr(ptr);
	mSelectPic._SetRootPtr(ptr);
	mPlusPic._SetRootPtr(ptr);
	mMinusPic._SetRootPtr(ptr);
}

//============================================================================================================
// Texture used by the font
//============================================================================================================

const ITexture* UITreeView::GetFontTexture()
{
	if (mFont == 0) mFont = mUI->GetDefaultFont();	
	return ((mFont != 0) ? mFont->GetTexture() : 0);
}

//============================================================================================================
// Texture used by the skin
//============================================================================================================

const ITexture* UITreeView::GetSkinTexture()
{
	const UISkin* skin = GetSkin();

	if (skin == 0)
	{
		skin = mUI->GetDefaultSkin();
		if (skin == 0) return 0;
		SetSkin(skin);
	}
	return skin->GetTexture();
}

//============================================================================================================
// Changes the font used by the label
//============================================================================================================

void UITreeView::SetFont (const IFont* font)
{
	if (mFont != font)
	{
		if (mFont != 0) SetDirty();
		mFont = const_cast<IFont*>(font);
		if (mFont != 0) SetDirty();
	}
}

//============================================================================================================
// Skin used by the widget
//============================================================================================================

void UITreeView::SetSkin (const UISkin* skin)
{
	if (GetSkin() != skin)
	{
		if (GetSkin() != 0) SetDirty();

		mSelectPic.Set	(skin, "Generic Highlight", false);
		mPlusPic.Set	(skin, mPrefix + ": Plus", false);
		mMinusPic.Set	(skin, mPrefix + ": Minus", false);

		if (GetSkin() != 0) SetDirty();
	}
}

//============================================================================================================
// Prefix used by the widget when looking up the associated UI Faces
//============================================================================================================

void UITreeView::SetPrefix (const String& s)
{
	if (GetSkin() != 0 && mPrefix != s)
	{
		mPrefix = s;
		mPlusPic.SetFace (mPrefix + ": Plus",  false);
		mMinusPic.SetFace(mPrefix + ": Minus", false);
		SetDirty();
	}
}

//============================================================================================================
// Changes the widget's background color
//============================================================================================================

void UITreeView::SetBackColor (const Color4ub& c)
{
	mSelectPic.SetBackColor(c);
	mPlusPic.SetBackColor(c);
	mMinusPic.SetBackColor(c);
}

//============================================================================================================
// INTERNAL: Prints all visible text to the draw queue
//============================================================================================================

void UITreeView::_PrintText (PrintParams& params)
{
	float size = mFont->GetSize();
	TreeNode* root = params.root;

	if (params.v0.y + size <= params.v1.y)
	{
		params.v0.x += size;

		if (params.start > 0)
		{
			--params.start;
		}
		else
		{
			uint width = mFont->CountChars(root->mTag, Float::RoundToUInt(params.v1.x - params.v0.x));

			// Print the shadow first
			if (mShadowColor.a > 0) mFont->Print(params.queue->mVertices, params.v0 + 1.0f,
				Color4ub(mShadowColor, mRegion.GetCalculatedAlpha()), root->mTag, 0, width);

			// Print the text
			mFont->Print(params.queue->mVertices, params.v0, Color4ub(mTextColor,
				mRegion.GetCalculatedAlpha()), root->mTag, 0, width);

			params.v0.y += size + mPadding;
		}

		if (params.root->mFlags.Get(1 << 31))
		{
			if (params.v0.x + size <= params.v1.x)
			{
				FOREACH(i, root->mChildren)
				{
					if (params.v0.y + size > params.v1.y) break;
					params.root = &root->mChildren[i];
					_PrintText(params);
				}
			}
		}
		params.v0.x -= size;
	}
}

//============================================================================================================
// INTERNAL: rints all visible symbols to the draw queue
//============================================================================================================

void UITreeView::_PrintSymbols (PrintParams& params)
{
	float size = mFont->GetSize();
	TreeNode* root = params.root;

	if (params.v0.y + size <= params.v1.y)
	{
		if (params.start > 0)
		{
			--params.start;
		}
		else
		{
			// If the node is currently selected, it should have a border around it
			if (root == mSelection)
			{
				UIRegion& rgn = mSelectPic.GetRegion();
				rgn.SetTop   (0.0f, params.v0.y);
				rgn.SetBottom(0.0f, params.v0.y + size);
				rgn.Update(mRegion, false, true);
				mSelectPic.OnFill(params.queue);
			}

			// If the node has children it needs a symbol -- plus or minus
			if (root->HasChildren())
			{
				if (params.root->mFlags.Get(1 << 31))
				{
					UIRegion& rgn = mMinusPic.GetRegion();
					rgn.SetTop	 (0.0f, params.v0.y);
					rgn.SetBottom(0.0f, params.v0.y + size);
					rgn.SetLeft	 (0.0f, params.v0.x);
					rgn.SetRight (0.0f, params.v0.x + size);
					rgn.Update(mRegion, false, true);
					mMinusPic.OnFill(params.queue);
				}
				else
				{
					UIRegion& rgn = mPlusPic.GetRegion();
					rgn.SetTop	 (0.0f, params.v0.y);
					rgn.SetBottom(0.0f, params.v0.y + size );
					rgn.SetLeft	 (0.0f, params.v0.x);
					rgn.SetRight (0.0f, params.v0.x + size);
					rgn.Update(mRegion, false, true);
					mPlusPic.OnFill(params.queue);
				}
			}

			// Move on to the next line
			params.v0.y += size + mPadding;
		}

		// Recurse through children if their nodes have been expanded
		if (root->HasChildren() && params.root->mFlags.Get(1 << 31))
		{
			params.v0.x += size;

			if (params.v0.x + size <= params.v1.x)
			{
				FOREACH(i, root->mChildren)
				{
					if (params.v0.y + size > params.v1.y) break;
					params.root = &root->mChildren[i];
					_PrintSymbols(params);
				}
			}
			params.v0.x -= size;
		}
	}
}

//============================================================================================================
// INTERNAL: Retrieves the node under the specified Y position
//============================================================================================================

TreeNode* UITreeView::_GetNode (PrintParams& params, float y)
{
	float size = mFont->GetSize();
	float paddedSize = size + mPadding;
	TreeNode* root = params.root;

	if (params.v0.y + size <= params.v1.y)
	{
		if (params.start > 0)
		{
			--params.start;
		}
		else
		{
			if (y < params.v0.y) return 0;

			if (y <= params.v0.y + paddedSize)
			{
				params.v1.y = params.v0.y + size;
				return root;
			}
			params.v0.y += paddedSize;
		}

		if (params.root->mFlags.Get(1 << 31))
		{
			FOREACH(i, root->mChildren)
			{
				if (params.v0.y + size > params.v1.y) break;
				params.root = &root->mChildren[i];
				TreeNode* retVal = _GetNode(params, y);
				if (retVal != 0) return retVal;
			}
		}
	}
	return 0;
}

//============================================================================================================
// Marks the associated queue as needing to be rebuilt
//============================================================================================================

void UITreeView::SetDirty()
{
	const ITexture* tex = GetFontTexture();
	if (tex != 0) OnDirty(tex, mLayer + 1);

	tex = GetSkinTexture();
	if (tex != 0) OnDirty(tex, mLayer);
}

//============================================================================================================
// Notification of texture having changed
//============================================================================================================

void UITreeView::OnTextureChanged (const ITexture* ptr)
{
	mSelectPic.OnTextureChanged(ptr);
	mPlusPic.OnTextureChanged(ptr);
	mMinusPic.OnTextureChanged(ptr);
}

//============================================================================================================
// Set the default font if one hasn't been chosen already
//============================================================================================================

bool UITreeView::OnUpdate (bool dimensionsChanged)
{
	if (GetFont() == 0) SetFont(mUI->GetDefaultFont());
	if (GetSkin() == 0) SetSkin(mUI->GetDefaultSkin());
	mSelectPic.Update(mRegion, dimensionsChanged, true);
	return false;
}

//============================================================================================================
// Fill the draw queue
//============================================================================================================

void UITreeView::OnFill (UIQueue* queue)
{
	if (queue->mTex == 0 || !mRoot.HasChildren()) return;

	const ITexture* fontTex = GetFontTexture();
	const ITexture* skinTex = GetSkinTexture();

	if (queue->mLayer == mLayer && queue->mTex == skinTex)
	{
		uint count (0);

		// Count the number of lines that would be displayed if we printed the entire tree as-is
		FOREACH(i, mRoot.mChildren) count += CountLines(mRoot.mChildren[i]);

		if (count > 0)
		{
			// Ensure we don't go past the end of the viewable tree
			if (mStart >= count) mStart = count - 1;
		}
		else
		{
			// Start at the beginning
			mStart = 0;
		}

		PrintParams params;
		params.queue	= queue;
		params.v0.x		= 0.0f;
		params.v0.y		= 0.0f;
		params.v1.x		= mRegion.GetCalculatedWidth();
		params.v1.y		= mRegion.GetCalculatedHeight();
		params.start	= mStart;

		// Print all symbols
		FOREACH(i, mRoot.mChildren)
		{
			params.root = &mRoot.mChildren[i];
			_PrintSymbols(params);
		}
	}

	// If it's time to draw the text, let's do that
	else if (queue->mLayer == mLayer + 1 && queue->mTex == fontTex)
	{
		PrintParams params;
		params.queue	= queue;
		params.v0.x		= mRegion.GetCalculatedLeft();
		params.v0.y		= mRegion.GetCalculatedTop();
		params.v1.x		= mRegion.GetCalculatedRight();
		params.v1.y		= mRegion.GetCalculatedBottom();
		params.start	= mStart;

		// Print all visible text
		FOREACH(i, mRoot.mChildren)
		{
			params.root = &mRoot.mChildren[i];
			_PrintText(params);
		}
	}
}

//============================================================================================================
// Respond to mouse wheel scroll event
//============================================================================================================

void UITreeView::OnScroll (const Vector2i& pos, float delta)
{
	const ITexture* tex = GetFontTexture();

	if (tex != 0)
	{
		float start = (float)mStart - delta;
		if (start < 0.0f) start = 0.0f;
		mStart = Float::RoundToUInt(start);
		SetDirty();
	}
	else
	{
		mStart = 0;
	}
	UIWidget::OnScroll(pos, delta);
}

//============================================================================================================
// Respond to key events
//============================================================================================================

void UITreeView::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (mRegion.Contains(pos))
	{
		bool dblClick = false;

		// Left mouse button should handle double-clicks
		if (!isDown && key == Key::MouseLeft)
		{
			ulong ms = Time::GetMilliseconds();
			if ((ms - mClickStamp) < 250) dblClick = true;
			mClickStamp = ms;
		}

		const ITexture* tex = GetFontTexture();

		if (tex != 0)
		{
			TreeNode* node (0);

			PrintParams params;
			mFont			= mFont;
			params.v0.y		= mRegion.GetCalculatedTop();
			params.v1.y		= mRegion.GetCalculatedBottom();
			params.start	= mStart;

			if (key == Key::MouseLeft && !isDown)
			{
				FOREACH(i, mRoot.mChildren)
				{
					params.root = &mRoot.mChildren[i];
					node = _GetNode(params, pos.y);

					if (node != 0)
					{
						// Double-click should expand the node
						if (dblClick)
						{
							if (node->HasChildren())
							{
								node->mFlags.Set(1 << 31, !node->mFlags.Get(1 << 31));
								SetDirty();
							}
						}
						else
						{
							SetSelection(node);
						}
						break;
					}
				}

				// Clear the selection if no node was found
				if (node == 0) SetSelection(0);
			}
		}
	}
	UIWidget::OnKeyPress(pos, key, isDown);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UITreeView::OnSerializeTo (TreeNode& root) const
{
	if (mPrefix != ClassID())		root.AddChild("Prefix",			mPrefix);
	if (GetSkin() != 0)				root.AddChild("Skin",			GetSkin()->GetName());
	if (GetFont() != 0)				root.AddChild(IFont::ClassID(), GetFont()->GetName());

	root.AddChild("Padding",		(int)mPadding);
	root.AddChild("Back Color",		GetBackColor());
	root.AddChild("Text Color",		GetTextColor());
	root.AddChild("Shadow Color",	GetShadowColor());
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UITreeView::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Prefix")
	{
		SetPrefix(node.mValue.AsString());
	}
	else if (node.mTag == "Skin")
	{
		SetSkin(mUI->GetSkin(node.mValue.AsString()));
	}
	else if (node.mTag == "Font")
	{
		SetFont(mUI->GetFont(node.mValue.AsString()));
	}
	else if (node.mTag == "Padding")
	{
		int val;
		if (node.mValue >> val) SetPadding((char)val);
	}
	else if (node.mTag == "Back Color")
	{
		Color4ub c;
		if (node.mValue >> c) SetBackColor(c);
	}
	else if (node.mTag == "Text Color")
	{
		Color4ub c;
		if (node.mValue >> c) SetTextColor(c);
	}
	else if (node.mTag == "Shadow Color")
	{
		Color4ub c;
		if (node.mValue >> c) SetShadowColor(c);
	}
	else return false;
	return true;
}