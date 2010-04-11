#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Script that plays the specified animations on the model instance it's attached to
//============================================================================================================

class OSPlayAnimations : public Script
{
	Array<String>	mAnims;

public:

	R5_DECLARE_INHERITED_CLASS("OSPlayAnimations", OSPlayAnimations, Script, Script);

	const Array<String>& Get() const { return mAnims; }
	void Set (const Array<String>& anims);

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const;
	virtual void OnSerializeFrom(const TreeNode& node);
};