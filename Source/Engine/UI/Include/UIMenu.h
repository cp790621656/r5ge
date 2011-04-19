#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Drop-down menu, based on the animated button class
// Author: Michael Lyashenko
//============================================================================================================

class UIMenu : public UIAnimatedButton
{
public:

	typedef Array<String> Entries;
	typedef UIAnimatedButton BaseClass;

protected:

	String	mMenuFace;			// Face used by the menu
	Entries	mEntries;			// Menu of drop-down menu entries
	bool	mSetTextOnSelect;	// If 'true', the text property will be updated on drop-down selection

public:

	UIMenu() : mSetTextOnSelect(false) {}

	// Drop-down list entry manipulation
	const Entries& GetAllEntries() const	{ return mEntries; }
	void ClearAllEntries() { mEntries.Clear(); }
	void AddEntry (const String& entry);
	void SetEntries (const Entries& entries);

protected:

	// Shows or hides the popup menu
	UIContext* _ShowMenu();
	UIContext* _HideMenu();

private:

	// Context menu callbacks
	void _OnContextFocus (UIWidget* widget, bool hasFocus);
	void _OnContextValue (UIWidget* widget);

public:

	// Respond to state changes
	virtual bool SetState (uint state, bool val);

protected:

	// Menu items are always left-aligned
	virtual uint _GetMenuItemAlignment() { return UILabel::Alignment::Left; }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIMenu", UIMenu, UIAnimatedButton, UIWidget);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};