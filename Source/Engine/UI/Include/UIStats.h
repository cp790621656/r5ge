#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Debugger widget -- contains various run-time statistics
// Author: Michael Lyashenko
//============================================================================================================

class UIStats : public UIAnimatedFrame
{
private:

	bool			mIsDirty;
	Color4ub		mShadow;
	Array<UILabel*>	mLabels;

	// INTERNAL: Adds a new label to the list
	UILabel* _AddLabel (const String& name);

public:

	UIStats() : mIsDirty(false) {}

	R5_DECLARE_INHERITED_CLASS("UIStats", UIStats, UIFrame, UIWidget);

	// Font is the only thing that can be changed
	const IFont* GetFont() const { return mLabels.IsValid() ? mLabels[0]->GetFont() : 0; }
	void SetFont (const IFont* font) { for (uint i = mLabels.GetSize(); i > 0; ) mLabels[--i]->SetFont(font); }

	// Color of the drop-down shadow
	const Color4ub& GetShadowColor() const { return mShadow; }
	void SetShadowColor (const Color4ub& c) { mShadow = c; }

	// Function called after the parent and root have been set
	virtual void OnInit();

	// Change the layer of the labels
	virtual void OnLayerChanged();

	// Only the update event is necessary for this widget
	virtual bool OnUpdate (bool dimensionsChanged);

protected:

	// Serialization
	virtual void OnSerializeTo (TreeNode& node) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};