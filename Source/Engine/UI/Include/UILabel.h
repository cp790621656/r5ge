#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Simple left/center/right-aligned text label
// Author: Michael Lyashenko
//============================================================================================================

class UILabel : public UIBasicLabel
{
public:

	struct Alignment
	{
		enum
		{
			Left	= 0,
			Center	= 1,
			Right	= 2,
		};
	};

protected:

	char  mAlignment;

public:

	UILabel() : mAlignment(Alignment::Left) {}

	char GetAlignment()	const { return mAlignment; }
	void SetAlignment (char alignment);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UILabel", UILabel, UIBasicLabel, UIWidget);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};