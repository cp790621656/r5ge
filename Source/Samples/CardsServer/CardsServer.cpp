//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// CardsClient: Networked game of cards (A-hole)
//============================================================================================================

#include "../../Engine/Serialization/Include/_All.h"
#include "../../Engine/Network/Include/_All.h"
using namespace R5;

const char* g_description[] = {"3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace", "2", "Wildcard"};

//============================================================================================================

struct Card
{
	byte value;	// For sorting purposes
	byte color;	// Hearts, Diamonds, Clubs, Spades

	operator ushort() const { return ((((ushort)value) << 8) | color); }
	void operator = (ushort val) { value = (byte)(val >> 8); color = (byte)(val & 0xF); }

	bool operator < (const Card& c)
	{
		if (value  < c.value) return true;
		if (value == c.value) return color < c.color;
		return false;
	}

	void Set (char val, byte c)
	{
		value = val;
		color = c;
	}

	const char* GetName() const { return g_description[value]; }
};

//============================================================================================================

struct Player
{
	uint		mSocket;
	String		mName;
	Array<Card> mHand;
	Memory		mIn;
};

//============================================================================================================

class Server
{
	typedef Array<Card> Hand;

	Network					mNet;		// Network library
	PointerArray<Player>	mPlayers;	// All connected players
	Array<uint>				mSockets;	// List of authenticated sockets
	uint					mListen;	// Listen socket

	Array<Card>				mDeck;		// Current deck of cards
	PointerArray<Hand>		mHands;		// Played hands
	Memory					mOut;		// Outgoing memory buffer
	TreeNode				mRoot;		// Outgoing TreeNode
	Random					mRand;		// Random number generator
	Player*					mCurrent;	// Player whos turn it is right now
	TreeNode				mTable;		// Current contents of the table

public:

	Server() : mListen(0), mCurrent(0) {}

	// New connection notification
	void OnConnect (const Network::Address& addr, uint socketId, VoidPtr& ptr, const String& msg);

	// Connection has been closed notification
	void OnClose (const Network::Address& addr, uint socketId, VoidPtr& ptr);

	// Receive new packet
	void OnReceive (const Network::Address& addr, uint socketId, VoidPtr& ptr, const byte* data, uint size, Thread::IDType threadId);

public:

	// Locks the buffer, starts the send process
	void BeginSend();

	// Sends the packet to the specified socket
	void EndSend (uint socketID);

	// Initialize the deck
	void Init (uint decks);

	// Process the player's buffered data
	bool Process (Player& player);

	// Process the player's packet
	bool ProcessPacket (Player& player, const byte* buffer, uint size);

	// Starts a new game
	void StartGame();

	// Send out a notification to all players
	void NotifyPlayers (Player* target, const String& tag, const String& targetMessage, const String& othersMessage);

	// Play the specified hand
	bool Play (const Array<ushort>& cards);

	// Advance the game to the next player
	void AdvanceGame();

	// Send the list of players to everyone
	void SendPlayerList();

	// Send the player's hand to the specified player
	void SendCards (Player* player);

	// Send the entire table's contents to the specified player
	void SendTable (Player* player);

	// Send out a text message to the player
	void SendMessage (Player* player, const String& msg);
};

//============================================================================================================
// New connection notification
//============================================================================================================

void Server::OnConnect (const Network::Address& addr, uint socketId, VoidPtr& ptr, const String& msg)
{
	mPlayers.Lock();
	{
		Player* player = new Player();
		player->mSocket = socketId;
		mPlayers.Expand() = player;
		ptr = player;
	}
	mPlayers.Unlock();
}

//============================================================================================================
// Connection has been closed notification
//============================================================================================================

void Server::OnClose (const Network::Address& addr, uint socketId, VoidPtr& ptr)
{
	Player* player = (Player*)ptr;

	if (player != 0)
	{
		printf("%s has disconnected (%s)\n", player->mName.GetBuffer(), addr.ToString().GetBuffer());

		mSockets.Lock();
		mSockets.Remove(socketId);
		mSockets.Unlock();

		mPlayers.Lock();
		{
			mPlayers.Remove(player);

			if (player->mHand.IsValid())
			{
				FOREACH(i, player->mHand)
				{
					mDeck.Expand() = player->mHand[i];
				}
			}
			delete player;
		}
		mPlayers.Unlock();
		SendPlayerList();
	}
}

//============================================================================================================
// Receive new packet
//============================================================================================================

void Server::OnReceive (const Network::Address& addr, uint socketId, VoidPtr& ptr, const byte* data, uint size, Thread::IDType threadId)
{
	Player* player = (Player*)ptr;

	if (player != 0)
	{
		player->mIn.Lock();
		{
			player->mIn.Append(data, size);
			Process(*player);
		}
		player->mIn.Unlock();
	}
}

//============================================================================================================
// Locks the buffer, starts the send process
//============================================================================================================

void Server::BeginSend()
{
	mOut.Lock();
	mOut.Clear();
	mOut.Expand(4);
	mRoot.Release();
}

//============================================================================================================
// Sends the packet to the specified socket
//============================================================================================================

void Server::EndSend (uint socketID)
{
	mRoot.SerializeTo(mOut);

	uint size = mOut.GetSize();
	byte* buffer = mOut.GetBuffer();
	*((uint32*)buffer) = (uint32)(size - 4);

	if (socketID != 0)
	{
		mNet.Send(buffer, size, socketID);
	}
	else
	{
		mSockets.Lock();
		mNet.Send(buffer, size, mSockets);
		mSockets.Unlock();
	}
	mOut.Unlock();
}

//============================================================================================================
// Initialize the deck
//============================================================================================================

void Server::Init (uint decks)
{
	if (decks == 0) decks = 1;
	if (decks > 3) decks = 3;

	puts("Cards Server v.1.0.0");

	// Create the cards
	for (uint a = 0; a < decks; ++a)
	{
		for (uint b = 0; b < 13; ++b)
		{
			for (uint c = 0; c < 4; ++c)
			{
				mDeck.Expand().Set(b, c);
			}
		}
		mDeck.Expand().Set(13, 0);
		mDeck.Expand().Set(13, 1);
	}

	// Bind the network listener callbacks
	mNet.SetOnConnect(bind(&Server::OnConnect, this));
	mNet.SetOnClose	 (bind(&Server::OnClose,	this));
	mNet.SetOnReceive(bind(&Server::OnReceive, this));
	mListen = mNet.Listen(3574);
	mNet.SpawnWorkerThread();
}

//============================================================================================================
// Process the player's buffered data
//============================================================================================================

bool Server::Process (Player& player)
{
	const byte* buffer = player.mIn.GetBuffer();
	uint size = player.mIn.GetSize();
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
		int retVal = ProcessPacket(player, buffer, packetSize);

		// Non-positive return value means something failed
		if (retVal < 1)
		{
			if (retVal == -1) printf("ERROR: Failed to parse a packet of size %u\n", packetSize);
			return false;
		}

		// Move on to the next packet
		buffer	+= packetSize;
		size	-= packetSize;
	}
	// Remove the processed data
	if (size < original) player.mIn.Remove(original - size);
	return true;
}

//============================================================================================================
// Process the player's packet
//============================================================================================================

bool Server::ProcessPacket (Player& player, const byte* buffer, uint size)
{
	TreeNode root;
	if (!root.SerializeFrom(buffer, size)) return false;

	if (root.mTag == "Name")
	{
		player.mName = root.mValue.AsString();

		if (player.mName.IsValid())
		{
			mSockets.AddUnique(player.mSocket);
			printf("%s has joined the game.\n", player.mName.GetBuffer());
			SendPlayerList();
		}
	}
	else if (root.mTag == "Play")
	{
		if (mListen == 0)
		{
			// Ignore players if it's not their turn, just to be safe
			if (&player != mCurrent) return true;
			if (!root.mValue.IsUShortArray()) return false;

			const Array<ushort>& arr = root.mValue.AsUShortArray();

			// The list of cards being played should come as the node's value
			if (arr.IsEmpty() || Play(arr))
			{
				// Save the table's current state
				mTable = root;

				// Send the updated table's contents to all players
				mPlayers.Lock();
				FOREACH(i, mPlayers) SendTable(mPlayers[i]);
				mPlayers.Unlock();

				// Advance the game to the next player
				AdvanceGame();
			}
			else
			{
				SendMessage(&player, "[FF0000]You can't play these cards.");
			}
		}
		else
		{
			mNet.Close(mListen);
			mListen = 0;
			StartGame();
		}
	}
	return true;
}

//============================================================================================================
// Starts a new game
//============================================================================================================

void Server::StartGame()
{
	Time::Update();
	mRand.SetSeed((uint)Time::GetMilliseconds());

	// Don't accept any more connections
	Array<uint> boot;

	// Find all players who don't have a name
	mPlayers.Lock();
	{
		FOREACH(i, mPlayers)
		{
			Player* p = mPlayers[i];
			if (p->mName.IsEmpty()) boot.Expand() = p->mSocket;
		}
	}
	mPlayers.Unlock();

	// Disconnect all players who still don't have a name
	FOREACH(i, boot) mNet.Close(boot[i]);

	// Send all players their cards
	mPlayers.Lock();
	{
		mCurrent = 0;

		// Deal cards
		while (mDeck.IsValid())
		{
			FOREACH(i, mPlayers)
			{
				uint index = mRand.GenerateUint() % mDeck.GetSize();
				const Card& card = mDeck[index];

				// 3 of clubs starts the game
				if (card.value == 0 && card.color == 2)
				{
					mCurrent = mPlayers[i];
				}

				mPlayers[i]->mHand.Expand() = card;
				mDeck.RemoveAt(index);
				if (mDeck.IsEmpty()) break;
			}
		}

		// Clear the table
		mTable.Release();

		// Send cards to players
		FOREACH(i, mPlayers)
		{
			Player* player = mPlayers[i];
			SendCards(player);
			SendTable(player);
		}
	}
	mPlayers.Unlock();

	// Notify the players of what's going on
	if (mCurrent != 0)
	{
		NotifyPlayers(mCurrent, "Play", "[FF9966]You[-] have the 3 of clubs and get to start.",
			String("[FF9966]%s[-] has the 3 of clubs and gets to start.", mCurrent->mName.GetBuffer()));
	}
}

//============================================================================================================
// Send out a notification to all players
//============================================================================================================

void Server::NotifyPlayers (Player* target, const String& tag, const String& targetMessage, const String& othersMessage)
{
	mPlayers.Lock();
	{
		FOREACH(i, mPlayers)
		{
			Player* player = mPlayers[i];

			if (player == target)
			{
				if (targetMessage.IsValid())
				{
					BeginSend();
					{
						mRoot.mTag = tag;
						mRoot.mValue = targetMessage;
					}
					EndSend(player->mSocket);
				}
			}
			else if (othersMessage.IsValid())
			{
				BeginSend();
				{
					mRoot.mTag = "Message";
					mRoot.mValue = othersMessage;
				}
				EndSend(player->mSocket);
			}
		}
	}
	mPlayers.Unlock();
}

//============================================================================================================
// Play the specified hand
//============================================================================================================

bool Server::Play (const Array<ushort>& cards)
{
	String msg (mCurrent->mName);

	if (cards.IsEmpty())
	{
		msg << " has passed";
	}
	else
	{
		msg << " has played: ";

		FOREACH(i, cards)
		{
			Card card;
			card = cards[i];
			if (i != 0) msg << ", ";
			msg << g_description[card.value];
		}
	}
	SendMessage(0, msg.GetBuffer());
	return true;
}

//============================================================================================================
// Advance the game to the next player
//============================================================================================================

void Server::AdvanceGame()
{
	bool found = false;
	Player* next (0);

	mPlayers.Lock();
	{
		FOREACH(i, mPlayers)
		{
			Player* player = mPlayers[i];

			if (found && player->mHand.IsValid())
			{
				next = mPlayers[i];
				break;
			}
			else if (player == mCurrent)
			{
				found = true;
			}
		}

		if (next == 0)
		{
			FOREACH(i, mPlayers)
			{
				Player* player = mPlayers[i];

				if (player->mHand.IsValid())
				{
					next = player;
					break;
				}
			}
		}
	}
	mPlayers.Unlock();

	if (next == 0 || next == mCurrent)
	{
		SendMessage(0, "Game over!");
	}
	else
	{
		mCurrent = next;
		NotifyPlayers(mCurrent, "Play", "[FF9966]Your turn!", String("[FF9966]%s[-]'s turn.",
					mCurrent->mName.GetBuffer()));
	}
}

//============================================================================================================
// Send the list of players to everyone
//============================================================================================================

void Server::SendPlayerList()
{
	mPlayers.Lock();
	{
		BeginSend();
		{
			mRoot.mTag = "Players";

			FOREACH(i, mPlayers)
			{
				const Player* p = mPlayers[i];

				if (p->mName.IsValid())
				{
					mRoot.AddChild("Name", p->mName);
				}
			}
		}
		EndSend(0);
	}
	mPlayers.Unlock();
}

//============================================================================================================
// Send the player's hand to the specified player
//============================================================================================================

void Server::SendCards (Player* player)
{
	BeginSend();
	{
		mRoot.mTag = "Player Hand";
		Array<ushort>& values = mRoot.mValue.ToUShortArray();

		FOREACH(i, player->mHand)
		{
			const Card& card = player->mHand[i];
			values.Expand() = card;
		}
	}
	EndSend(player->mSocket);
}

//============================================================================================================
// Send the entire table's contents to the specified player
//============================================================================================================

void Server::SendTable (Player* player)
{
	BeginSend();
	mRoot = mTable;
	mRoot.mTag = "Table";
	EndSend(player->mSocket);
}

//============================================================================================================
// Send out a text message to the player
//============================================================================================================

void Server::SendMessage (Player* player, const String& msg)
{
	BeginSend();
	mRoot.mTag = "Message";
	mRoot.mValue = msg;
	EndSend(player != 0 ? player->mSocket : 0);
}

//============================================================================================================
// Application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
	uint decks = 1;

	if (argc > 1)
	{
		String temp (argv[1]);
		temp >> decks;
	}

	Server app;
	app.Init(decks);
	getchar();
	return 0;
}