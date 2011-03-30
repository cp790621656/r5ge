#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Widget designed to visualize TreeNode hierarchies
//============================================================================================================

class UITreeView : public UIWidget
{
protected:

	// Group of parameters passed to the print functions
	struct PrintParams
	{
		UIQueue*	queue;
		TreeNode*	root;
		Vector2f	v0;
		Vector2f	v1;
		uint		start;

		PrintParams() : queue(0), root(0), start(0) {}
	};

protected:

	IFont*			mFont;
	TreeNode		mRoot;
	char			mPadding;
	uint			mStart;
	Color4ub		mTextColor;
	Color4ub		mShadowColor;
	ulong			mClickStamp;
	TreeNode*		mSelection;
	String			mPrefix;

	UISubPicture	mSelectPic;
	UISubPicture	mPlusPic;
	UISubPicture	mMinusPic;

	UITreeView();

	// Texture used by the font
	const ITexture* GetFontTexture();
	const ITexture* GetSkinTexture();

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UITreeView", UITreeView, UIWidget, UIWidget);

	// Font used by the widget
	const IFont* GetFont() const { return mFont; }
	void SetFont (const IFont* font);

	// Skin used by the widget
	const UISkin* GetSkin() const { return mSelectPic.GetSkin(); }
	void SetSkin (const UISkin* skin);

	// Prefix used by the widget when looking up the associated UI Faces
	const String& GetPrefix() const { return mPrefix; }
	void SetPrefix (const String& s);

	// Padding between the lines, in pixels
	char GetPadding() const { return mPadding; }
	void SetPadding (char pixels) { if (mPadding != pixels) { mPadding = pixels; SetDirty(); } }

	// Initial text color
	const Color4ub& GetTextColor() const { return mTextColor; }
	void SetTextColor (const Color4ub& c) { if (mTextColor != c) { mTextColor = c; OnDirty(GetFontTexture()); } }

	// Background color
	const Color4ub& GetShadowColor() const { return mShadowColor; }
	void SetShadowColor (const Color4ub& c) { if (mShadowColor != c) { mShadowColor = c; OnDirty(GetFontTexture()); } }

	// Background color
	const Color4ub& GetBackColor() const { return mPlusPic.GetBackColor(); }
	void SetBackColor (const Color4ub& c);

	// Access to the entire tree structure
	// NOTE: Don't forget to call SetSelection(0) if you modify the tree's hierarchy via GetTree()!
	TreeNode& GetTree() { return mRoot; }
	const TreeNode& GetTree() const { return mRoot; }
	void SetTree (const TreeNode& root) { mRoot = root; SetSelection(0); }

	// Access to the current selection
	TreeNode* GetSelection() { return mSelection; }
	const TreeNode* GetSelection() const { return mSelection; }
	void SetSelection (TreeNode* node) { mSelection = node; OnValueChange(); SetDirty(); }

protected:

	// INTERNAL: Prints all visible text to the draw queue
	void _PrintText (PrintParams& params);

	// INTERNAL: Prints all visible symbols to the draw queue
	void _PrintSymbols (PrintParams& params);

	// INTERNAL: Retrieves the node under the specified Y position
	TreeNode* _GetNode (PrintParams& params, float y);

public:

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIWidget* ptr);
	virtual void _SetRootPtr   (UIManager* ptr);

	// Marks the visible list as dirty
	virtual void SetDirty();

	// Area functions
	virtual void OnTextureChanged (const ITexture* ptr);
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

	// Respond to events
	virtual void OnScroll (const Vector2i& pos, float delta);
	virtual void OnKeyPress (const Vector2i& pos, byte key, bool isDown);

	// Serialization
	virtual void OnSerializeTo (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};