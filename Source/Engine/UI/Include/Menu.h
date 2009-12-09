#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Drop-down menu, based on the animated button class
//============================================================================================================

class UIMenu : public UIAnimatedButton
{
public:

	typedef Array<String> Entries;
	typedef UIAnimatedButton BaseClass;

protected:

	String	mSelection;	// Last selected value in the drop-down list
	String	mMenuFace;	// Face used by the menu
	Entries	mEntries;	// Menu of drop-down menu entries

public:

	// Drop-down list entry manipulation
	const Entries& GetAllEntries() const	{ return mEntries; }
	void ClearAllEntries();
	void AddEntry (const String& entry);
	void SetEntries (const Entries& entries);

	// The last selected value
	virtual const String& GetLastSelection() const	{ return mSelection; }

protected:

	// Shows or hides the popup menu
	UIContext* _ShowMenu();
	UIContext* _HideMenu();

	// Overrideable function triggered when the value changes (used by the List widget)
	virtual void _OnValue (const String& val) {}

private:

	// Context menu callbacks
	bool _OnContextFocus (UIArea* area, bool hasFocus);
	bool _OnContextValue (UIArea* area);

public:

	// Respond to state changes
	virtual bool SetState	(uint state, bool val);
	//virtual bool OnFocus	(bool hasFocus);

protected:

	// Menu items are always left-aligned
	virtual uint _GetMenuItemAlignment() { return UILabel::Alignment::Left; }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Menu", UIMenu, UIAnimatedButton, UIArea);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;
};