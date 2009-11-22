#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Simple left/center/right-aligned text label
//============================================================================================================

class Label : public BasicLabel
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

	Label() : mAlignment(Alignment::Left) {}

	char GetAlignment()	const { return mAlignment; }
	void SetAlignment (char alignment);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Label", Label, BasicLabel, Area);

	// Called when a queue is being rebuilt
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root);
	virtual void CustomSerializeTo(TreeNode& root) const;
};