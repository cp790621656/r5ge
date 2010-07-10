#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Debugger widget -- contains various run-time statistics
//============================================================================================================

class UIStats : public UIAnimatedFrame
{
private:

	bool mIsDirty;
	Array<UILabel*>	mLabels;

	// INTERNAL: Adds a new label to the list
	UILabel* _AddLabel (const String& name);

public:

	UIStats() : mIsDirty(false) {}

	R5_DECLARE_INHERITED_CLASS("UIStats", UIStats, UIFrame, UIWidget);

	// Font is the only thing that can be changed
	const IFont* GetFont() const { return mLabels.IsValid() ? mLabels[0]->GetFont() : 0; }
	void SetFont (const IFont* font) { for (uint i = mLabels.GetSize(); i > 0; ) mLabels[--i]->SetFont(font); }

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