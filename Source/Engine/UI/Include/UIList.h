#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Drop-down list (or up, depending on space)
// Author: Michael Lyashenko
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
	R5_DECLARE_INHERITED_CLASS("UIList", UIList, UIMenu, UIWidget);

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