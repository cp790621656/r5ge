//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

String g_message;
uint g_allowDiscard = 0;
Array<Card> g_discard;

//============================================================================================================
// Ensure the script is attached to a button
//============================================================================================================

void USConnect::OnInit()
{
	mButton = R5_CAST(UIButton, mWidget);
	if (mButton == 0) DestroySelf();
}

//============================================================================================================
// Remove bound callbacks
//============================================================================================================

void USConnect::OnDestroy()
{
	mNet.SetOnConnect(0);
	mNet.SetOnClose	 (0);
	mNet.SetOnReceive(0);
	mNet.SetOnError	 (0);
	mNet.Disconnect();
}

//============================================================================================================
// Connect to the server
//============================================================================================================

void USConnect::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (!mIsConnected && key == Key::MouseLeft && isDown && mWidget->GetRegion().Contains(pos))
	{
		mAddress	= mWidget->GetUI()->FindWidget<UIInput>("Server Address");
		mName		= mWidget->GetUI()->FindWidget<UIInput>("Player Name");
		mPlay		= mWidget->GetUI()->FindWidget<UIButton>("Play Button");
		mHand		= mWidget->GetUI()->FindWidget<UIHighlight>("Player Hand");

		if (mAddress != 0 && mName != 0 && mPlay != 0 && mName->GetText().IsValid())
		{
			mAddress->SetEventHandling(UIWidget::EventHandling::None);
			mName->SetEventHandling(UIWidget::EventHandling::None);

			mButton->SetState(UIButton::State::Pressed,		false);
			mButton->SetState(UIButton::State::Enabled,		false);
			mButton->SetState(UIButton::State::Highlighted, false);

			mNet.SetOnConnect(bind(&USConnect::OnConnect,	this));
			mNet.SetOnClose	 (bind(&USConnect::OnClose,		this));
			mNet.SetOnReceive(bind(&USConnect::OnReceive,	this));
			mNet.SetOnError	 (bind(&USConnect::OnError,		this));
			mNet.Connect(mAddress->GetText());
			mNet.SpawnWorkerThread();

			mPlay->AddScript<USEventListener>()->SetOnKey(bind(&USConnect::OnPlay, this));
		}
	}
}

//============================================================================================================
// Send out messages when they arrive
//============================================================================================================

void USConnect::OnUpdate (bool areaChanged)
{
	if (g_message.IsValid())
	{
		BeginSend();
		mRoot.mTag = "Message";
		mRoot.mValue = g_message;
		EndSend();
		g_message.Clear();
	}

	// Enable or disable the play button depending on whether we can actually play
	bool canPlay = !g_allowDiscard && mMyTurn;

	if (mPlay != 0)
	{
		mPlay->SetState(UIButton::State::Enabled, canPlay);

		if (!canPlay)
		{
			mPlay->SetState(UIButton::State::Highlighted,	false);
			mPlay->SetState(UIButton::State::Pressed,		false);
		}
	}

	// Color the dock accordingly
	if (mHand != 0)
	{
		if (g_allowDiscard > 0)	mHand->SetBottomColor(Color4f(0.0f, 0.5f, 1.0f, 0.8f));
		else if (mMyTurn)		mHand->SetBottomColor(Color4f(0.5f, 1.0f, 0.0f, 0.8f));
		else					mHand->SetBottomColor(Color4f(0.0f, 0.0f, 0.0f, 0.8f));
	}

	// If we have cards to discard, notify the server
	if (g_discard.IsValid())
	{
		g_discard.Lock();
		{
			BeginSend();
			{
				mRoot.mTag = "Discard";
				Array<ushort>& arr = mRoot.mValue.ToUShortArray();
				FOREACH(i, g_discard) arr.Expand() = g_discard[i];
				g_discard.Clear();
			}
			EndSend();
		}
		g_discard.Unlock();
	}
}

//============================================================================================================
// Locks the buffer, starts the send process
//============================================================================================================

void USConnect::BeginSend()
{
	mOut.Lock();
	mOut.Clear();
	mOut.Expand(4);
	mRoot.Release();
}

//============================================================================================================
// Connection has been closed notification
//============================================================================================================

void USConnect::OnClose (const Network::Address& addr, uint socketId, VoidPtr& ptr)
{
	mIsConnected = false;

	USHand* hand = (mHand != 0 ? mHand->GetScript<USHand>() : 0);

	if (hand	 != 0) hand->Clear();
	if (mAddress != 0) mAddress->SetEventHandling(UIWidget::EventHandling::Normal);
	if (mName	 != 0) mName->SetEventHandling(UIWidget::EventHandling::Normal);

	mButton->SetState(UIButton::State::Enabled, true);
	USMessageLog::Show("Disconnected");
}

//============================================================================================================
// Receive new packet
//============================================================================================================

void USConnect::OnReceive (const Network::Address& addr, uint socketId, VoidPtr& ptr, const byte* data, uint size, Thread::IDType threadId)
{
	mIn.Append(data, size);

	const byte* buffer = mIn.GetBuffer();
	size = mIn.GetSize();

	uint original = size;
	uint packetSize;

	while (size > 4)
	{
		packetSize = (*(const uint32*)buffer);
		if (size < packetSize) break;

		// Skip past the packet size
		buffer += 4;
		size   -= 4;

		// Process this single packet
		int retVal = ProcessPacket(buffer, packetSize);

		// Non-positive return value means something failed
		if (retVal < 1)
		{
			if (retVal == -1) printf("ERROR: Failed to parse a packet of size %u\n", packetSize);
			return;
		}

		// Move on to the next packet
		buffer	+= packetSize;
		size	-= packetSize;
	}
	// Remove the processed data
	if (size < original) mIn.Remove(original - size);
	return;
}

//============================================================================================================
// Error notification
//============================================================================================================

void USConnect::OnError (const Network::Address& addr, uint socketId, VoidPtr& ptr, const char* message)
{
	OnClose(addr, socketId, ptr);
}

//============================================================================================================
// Sends the packet to the specified socket
//============================================================================================================

void USConnect::EndSend()
{
	mRoot.SerializeTo(mOut);
	uint size = mOut.GetSize();
	byte* buffer = mOut.GetBuffer();
	*((uint32*)buffer) = (uint32)(size - 4);
	mNet.Send(buffer, size);
	mOut.Unlock();
}

//============================================================================================================
// New connection notification
//============================================================================================================

void USConnect::OnConnect (const Network::Address& addr, uint socketId, VoidPtr& ptr, const String& msg)
{
	mIsConnected = true;

	BeginSend();
	{
		mRoot.mTag = "Name";
		mRoot.mValue = mName->GetText();
	}
	EndSend();
}

//============================================================================================================
// Callback triggered when the Play button is pressed
//============================================================================================================

void USConnect::OnPlay (UIWidget* widget, const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft && isDown && widget->GetRegion().Contains(pos))
	{
		UIWidget* table = GetTable();

		BeginSend();
		{
			TreeNode dummy;
			mTable->SerializeTo(dummy);
			mRoot = dummy.mChildren[0];
			mRoot.mTag = "Play";

			if (table != 0)
			{
				Array<ushort>& arr = mRoot.mValue.ToUShortArray();
				UIWidget::Children& children = table->GetAllChildren();

				FOREACH(i, children)
				{
					UIWidget* child = children[i];
					USCard* card = child->GetScript<USCard>();

					if (card != 0 && card->BelongsToPlayer())
					{
						arr.Expand() = card->GetCard();
					}
				}
			}
		}
		EndSend();
	}
}

//============================================================================================================
// Sets the hand's values given the specified TreeNode with an array
//============================================================================================================

void USConnect::SetHand (const TreeNode& root)
{
	LockUI();
	{
		USHand* hand = (mHand != 0 ? mHand->GetScript<USHand>() : 0);

		if (hand != 0)
		{
			hand->Clear();

			if (root.mValue.IsUShortArray())
			{
				const Array<ushort>& cards = root.mValue.AsUShortArray();

				FOREACH(i, cards)
				{
					ushort card = cards[i];
					hand->Add((byte)(card >> 8), (byte)(card & 0xF));
				}
			}
		}
	}
	UnlockUI();
}

//============================================================================================================
// Process the player's packet
//============================================================================================================

bool USConnect::ProcessPacket (const byte* buffer, uint size)
{
	TreeNode root;
	if (!root.SerializeFrom(buffer, size)) return false;

	if (root.mTag.Contains("Hand"))
	{
		g_allowDiscard = 0;
		SetHand(root);
	}
	else if (root.mTag == "Table")
	{
		mMyTurn = false;

		LockUI();
		{
			UIWidget* table = GetTable();

			if (table != 0)
			{
				table->DestroyAllWidgets();
				table->SerializeFrom(root);
			}
		}
		UnlockUI();
	}
	else if (root.mTag == "Play")
	{
		mMyTurn = true;
		USMessageLog::Show(root.mValue.AsString());
	}
	else if (root.mTag == "Message")
	{
		USMessageLog::Show(root.mValue.AsString());
	}
	else if (root.mTag == "Discard")
	{
		uint add;
		if (root.mValue >> add) g_allowDiscard += add;
	}
	else if (root.mTag == "Players")
	{
		LockUI();
		{
			UITextArea* text = mWidget->GetUI()->FindWidget<UITextArea>("Players");

			if (text != 0)
			{
				text->Clear();

				FOREACH(i, root.mChildren)
				{
					TreeNode& node = root.mChildren[i];

					if (node.mTag == "Name")
					{
						text->AddParagraph(node.mValue.AsString());
					}
				}
			}
		}
		UnlockUI();
	}
	else
	{
		String desc ("Unknown tag: ");
		desc << root.mTag;
		ASSERT(false, desc.GetBuffer());
	}
	return true;
}