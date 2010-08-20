#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Script attached to cards allowing the player to drag them around and drop them onto the table
//============================================================================================================

class USCard : public UIScript
{
	Card		mCard;
	UIButton*	mButton;
	bool		mBelongsToPlayer;
	bool		mDragging;

	USCard() : mButton(0), mBelongsToPlayer(false), mDragging(false) {}

public:

	R5_DECLARE_INHERITED_CLASS("USCard", USCard, UIScript, UIScript);

	const Card& GetCard() const { return mCard; }
	void SetCard (const Card& card) { mCard = card; mButton->SetBackColor(mCard.GetColor()); }

	// Whether the card can be dragged around by the player
	void SetBelongsToPlayer (bool val);
	void StartDragging (bool val) { mBelongsToPlayer = true; mDragging = val; }

	// Whether the card belongs to the player
	bool BelongsToPlayer() const { return mBelongsToPlayer; }

public:

	// Must be attached to a button
	virtual void OnInit();

	// Start / stop dragging
	virtual void OnKeyPress (const Vector2i& pos, byte key, bool isDown);

	// Drag the card
	virtual void OnMouseMove (const Vector2i& pos, const Vector2i& delta);
};