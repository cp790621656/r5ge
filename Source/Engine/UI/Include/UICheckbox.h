#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Checkbox (Slightly modified sticky button)
// Author: Michael Lyashenko
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
	UICheckbox() { mPrefix = ClassID(); SetSticky(true); SetAlignment(UILabel::Alignment::Left); }

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UICheckbox", UICheckbox, UIButton, UIWidget);

	// Whether the checkbox is checked
	bool IsChecked() const { return (mState & State::Checked) != 0; }
	void SetChecked (bool val) { SetState(State::Checked, val); }

	// Area functions
	virtual bool SetState (uint state, bool val);
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

protected:

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};