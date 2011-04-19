#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Player's hand
// Author: Michael Lyashenko
//============================================================================================================

class USHand : public UIScript
{
	Array<Card> mHand;
	bool		mReset;
	bool		mDrag;

	USHand() : mReset(false), mDrag(false) {}

public:

	R5_DECLARE_INHERITED_CLASS("USHand", USHand, UIScript, UIScript);

	// Add a new card to the hand
	void Add (byte value, byte color);

	// Remove all cards from the hand
	void Clear();

	// Whether the hand is valid
	bool IsValid() const { return mHand.IsValid(); }

	// If a reset was requested, recreate the visual representation of available cards
	virtual void OnUpdate (bool areaChanged);

	// Serialization
	virtual void OnSerializeTo (TreeNode& root) const;
	virtual void OnSerializeFrom (const TreeNode& node);
};