//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

//============================================================================================================

const char* g_description[] = {"3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "*"};

//============================================================================================================
// Add a new card to the hand
//============================================================================================================

void USHand::Add (byte value, byte color)
{
	mHand.Lock();
	{
		mHand.Expand().Set(value, color);
		mReset = true;
	}
	mHand.Unlock();
}

//============================================================================================================
// Remove all cards from the hand
//============================================================================================================

void USHand::Clear()
{
	mHand.Lock();
	{
		mHand.Clear();
		mReset = true;
	}
	mHand.Unlock();
}

//============================================================================================================
// If a reset was requested, recreate the visual representation of available cards
//============================================================================================================

void USHand::OnUpdate (bool areaChanged)
{
	if (mReset)
	{
		mReset = false;
		mWidget->DestroyAllWidgets();

		mHand.Lock();
		mHand.Sort();

		Random rand;
		Time::Update();
		rand.SetSeed((uint)Time::GetMilliseconds());
		rand.SetSeed(rand.GenerateUint());

		UIInput* playerName = mWidget->GetUI()->FindWidget<UIInput>("Player Name");
		ASSERT(playerName != 0, "Player name is missing?");

		FOREACH(i, mHand)
		{
			const Card& card = mHand[i];

			String name ("Card");
			if (playerName != 0) name = playerName->GetText();

			UIButton* btn = mWidget->AddWidget<UIButton>(String("%s %u",
				name.GetBuffer(), rand.GenerateUint()), false);

			btn->SetText(g_description[card.value]);
			btn->SetTextColor(Color4ub(0, 0, 0, 255));
			btn->SetLayer(5);

			UIRegion& rgn = btn->GetRegion();
			float offset = -0.5f * mHand.GetSize() * 30.0f + i * 30.0f;
			rgn.SetLeft(0.5f, offset - 15.0f);
			rgn.SetRight(0.5f, offset + 15.0f);
			rgn.SetTop(0.5f, -15.0f);
			rgn.SetBottom(0.5f, 15.0f);

			// The player should be able to drag their own cards
			USCard* script = btn->AddScript<USCard>();
			script->SetCard(card);
			if (mDrag) script->SetBelongsToPlayer(true);
		}
		mHand.Unlock();
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void USHand::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Draggable", mDrag);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void USHand::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Draggable") node.mValue >> mDrag;
}