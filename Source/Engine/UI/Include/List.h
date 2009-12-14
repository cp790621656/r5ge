#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Drop-down list (or up, depending on space)
//============================================================================================================

class UIList : public UIMenu
{
protected:

	UISubPicture	mSymbol;

public:

	const UIFace* GetSymbol() const { return mSymbol.GetFace();}

	void SetSymbol	(const String& face)	{ mSymbol.SetFace(face);	}
	void SetSkin	(const UISkin* skin)	{ UIMenu::SetSkin(skin); mSymbol.SetSkin(skin); }

protected:

	// Keep the label's alignment for list items
	virtual uint _GetMenuItemAlignment() { return mLabel.GetAlignment(); }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("List", UIList, UIMenu, UIArea);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIArea* ptr);
	virtual void _SetRootPtr   (UIRoot* ptr);

	// Area functions
	virtual void OnLayerChanged();
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
};