#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Checkbox (Slightly modified sticky button)
//============================================================================================================

class Checkbox : public Button
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
	Checkbox() { SetSticky(true); SetAlignment(Label::Alignment::Left); }

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Checkbox", Checkbox, Button, Area);

	// Area functions
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (Queue* queue);

protected:

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
	virtual void CustomSerializeTo (TreeNode& root) const;
};