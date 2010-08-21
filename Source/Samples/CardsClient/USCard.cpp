//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

extern uint g_allowDiscard;
extern Array<Card> g_discard;
extern ulong g_activityTime;

//============================================================================================================
// Whether the card can be dragged around by the player
//============================================================================================================

void USCard::SetBelongsToPlayer (bool val)
{
	mBelongsToPlayer = val;
	mButton->SetState(UIButton::State::Enabled, val);
}

//============================================================================================================
// Must be attached to a button
//============================================================================================================

void USCard::OnInit()
{
	mButton = R5_CAST(UIButton, mWidget);
	if (mButton == 0) DestroySelf();
}

//============================================================================================================
// Start / stop dragging
//============================================================================================================

void USCard::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (mBelongsToPlayer)
	{
		if (mDragging)
		{
			mDragging = false;

			UIWidget* table = mWidget->GetUI()->FindWidget<UIWidget>("Table");
			UIWidget* hand  = mWidget->GetUI()->FindWidget<UIWidget>("Player Hand");

			if (table != 0 && table->GetRegion().Contains(pos))
			{
				if (g_allowDiscard > 0)
				{
					--g_allowDiscard;
					g_discard.Lock();
					g_discard.Expand() = mCard;
					g_discard.Unlock();
					mWidget->DestroySelf();
				}
				else
				{
					mWidget->SetParent(table);
				}
			}
			else if (hand != 0 && hand->GetRegion().Contains(pos))
			{
				mWidget->SetParent(hand);
			}
		}
		else if (key == Key::MouseLeft && isDown)
		{
			// It must be the player's turn before they can drag cards
			UIManager* ui = mWidget->GetUI();
			UIButton* play = ui->FindWidget<UIButton>("Play Button");

			if (play == 0) return;
			if (g_allowDiscard == 0 && !play->GetState(UIButton::State::Enabled)) return;

			// The widget's parent will now be the root node
			mWidget->SetParent(&ui->GetRoot());
			mDragging = true;
		}
	}
}

//============================================================================================================
// Drag the card
//============================================================================================================

void USCard::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mDragging && mBelongsToPlayer)
	{
		g_activityTime = Time::GetMilliseconds();
		mWidget->Adjust(delta.x, delta.y, delta.x, delta.y);
	}
}