#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Script that plays the specified animations on the model instance it's attached to
// Author: Michael Lyashenko
//============================================================================================================

class OSPlayAnimations : public Script
{
protected:

	Array<String> mAnims;

	// Use the AddScript<> template to add new scripts
	OSPlayAnimations() {}

public:

	R5_DECLARE_INHERITED_CLASS("OSPlayAnimations", OSPlayAnimations, Script, Script);

	const Array<String>& Get() const { return mAnims; }
	void Set (const Array<String>& anims);

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const;
	virtual void OnSerializeFrom(const TreeNode& node);
};