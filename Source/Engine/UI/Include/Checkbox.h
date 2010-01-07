#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Checkbox (Slightly modified sticky button)
//============================================================================================================

class UICheckbox : public UIButton
{
public:

	struct State
	{
		enum
		{
			Enabled		= 1,
			Highlighted = 2,
			Checked		= 4
		};
	};

public:

	// Checkbox is essentially a sticky button
	UICheckbox() { SetSticky(true); SetAlignment(UILabel::Alignment::Left); }

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Checkbox", UICheckbox, UIButton, UIArea);

	// Area functions
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

protected:

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;
};