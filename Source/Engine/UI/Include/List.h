#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Drop-down list (or up, depending on space)
//============================================================================================================

class List : public Menu
{
protected:

	SubPicture	mSymbol;

public:

	const Face*		GetSymbol()	const		{ return mSymbol.GetFace();}
	void SetSymbol	(const String& face)	{ mSymbol.SetFace(face);	}
	void SetSkin	(const Skin* skin)		{ Menu::SetSkin(skin); mSymbol.SetSkin(skin); }

	virtual const String& GetLastSelection() const	{ return (mSelection.IsValid() ? mSelection : mLabel.GetText()); }

protected:

	// List selection should update the label
	virtual void _OnValue (const String& val);

	// Keep the label's alignment for list items
	virtual uint _GetMenuItemAlignment() { return mLabel.GetAlignment(); }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("List", List, Menu, Area);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (Area* ptr);
	virtual void _SetRootPtr   (Root* ptr);

	// Area functions
	virtual void OnLayerChanged();
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
};