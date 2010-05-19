#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple left/center/right-aligned text label
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
	R5_DECLARE_INHERITED_CLASS("Label", UILabel, UIBasicLabel, UIWidget);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};