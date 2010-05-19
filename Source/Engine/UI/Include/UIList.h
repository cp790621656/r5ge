#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Drop-down list (or up, depending on space)
//============================================================================================================

class UIList : public UIMenu
{
protected:

	UISubPicture	mSymbol;

public:

	// Drop-down list should always update its own text value on selection
	UIList() { mSetTextOnSelect = true; }

	const UIFace* GetSymbol() const { return mSymbol.GetFace();}

	void SetSymbol	(const String& face)	{ mSymbol.SetFace(face);	}
	void SetSkin	(const UISkin* skin)	{ UIMenu::SetSkin(skin); mSymbol.SetSkin(skin); }

protected:

	// Keep the label's alignment for list items
	virtual uint _GetMenuItemAlignment() { return mLabel.GetAlignment(); }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("List", UIList, UIMenu, UIWidget);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIWidget* ptr);
	virtual void _SetRootPtr   (UIManager* ptr);

	// Notification of texture change
	virtual void OnTextureChanged(const ITexture* ptr);

	// Area functions
	virtual void OnLayerChanged();
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
};