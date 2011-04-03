//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// CardsClient: Networked game of cards (A-hole)
//============================================================================================================

#include "../../Engine/Serialization/Include/_All.h"
#include "../../Engine/Network/Include/_All.h"
using namespace R5;

#define TWOCARD  12
#define WILDCARD 13

//============================================================================================================

const char* g_description[] =
{
	"Three",
	"Four",
	"Five",
	"Six",
	"Seven",
	"Eight",
	"Nine",
	"Ten",
	"Jack",
	"Queen",
	"King",
	"Ace",
	"Two",
	"Wildcard"
};

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
	//ulong		mTime;
};

//============================================================================================================

struct Move
{
	Player*	mPlayer;
	Card	mCard;
	uint	mCount;
};

//============================================================================================================

class Server
{
	Network					mNet;		// Network library
	PointerArray<Player>	mPlayers;	// All connected players
	Array<uint>				mSockets;	// List of authenticated sockets
	bool					mActive;	// Whether the game is in progress
	uint					mDeckCount;	// Number of decks we're playing with

	Array<Card>				mDeck;		// Current deck of cards
	Array<Move>				mMove;		// Played moves
	Array<Player*>			mWinners;	// List of winners
	Memory					mOut;		// Outgoing memory buffer
	TreeNode				mRoot;		// Outgoing TreeNode
	Random					mRand;		// Random number generator
	Player*					mCurrent;	// Player whos turn it is right now
	TreeNode				mTable;		// Current contents of the table
	uint					mMoveCount;	// How many rounds has it been since a discard

public:

	Server() : mActive(false), mCurrent(0), mMoveCount(0) {}

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
	uint Play (const Array<ushort>& cards, const TreeNode& table);

	// Advance the game to the next player
	void AdvanceGame (bool passed);

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
	Player* player = new Player();
	player->mSocket = socketId;
	mPlayers.Expand() = player;
	ptr = player;
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

		String msg (player->mName);

		if (mCurrent == player)
		{
			AdvanceGame(true);
		}

		mSockets.Remove(socketId);
		mWinners.Remove(player);

		FOREACH(i, mMove)
		{
			if (mMove[i].mPlayer == player)
			{
				mMove.RemoveAt(i);
				i = 0;
			}
		}

		if (player->mHand.IsValid())
		{
			FOREACH(i, player->mHand)
			{
				mDeck.Expand() = player->mHand[i];
			}
		}
		mPlayers.Delete(player);

		SendPlayerList();
		msg << " has abandoned the game.";
		SendMessage(0, msg.GetBuffer());

		if (mPlayers.GetSize() == 1)
		{
			mActive = false;

			FOREACH(i, mPlayers)
			{
				Player* p = mPlayers[i];

				p->mHand.Clear();
				SendCards(p);
				SendMessage(p, "You can start another game by pressing Play when everyone is connected.");
			}
		}
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
		//player->mTime = Time::GetMilliseconds();
		player->mIn.Append(data, size);
		Process(*player);
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
		mNet.Send(buffer, size, mSockets);
	}
	mOut.Unlock();
}

//============================================================================================================
// Initialize the deck
//============================================================================================================

void Server::Init (uint decks)
{
	mDeckCount = decks;
	if (mDeckCount < 1) mDeckCount = 1;
	if (mDeckCount > 2) mDeckCount = 2;

	puts("Cards Server v.1.1.0");

	// Bind the network listener callbacks
	mNet.SetOnConnect(bind(&Server::OnConnect,	this));
	mNet.SetOnClose	 (bind(&Server::OnClose,	this));
	mNet.SetOnReceive(bind(&Server::OnReceive,	this));
	mNet.Listen(3574);
	mNet.SpawnWorkerThread();
}

//============================================================================================================
// Process the player's buffered data
//============================================================================================================

bool Server::Process (Player& player)
{
	//FOREACH(i, mPlayers)
	//{
	//	Player* p = mPlayers[i];

	//	if (p == mCurrent)
	//	{
	//		if (mCurrent->mTime + 60000 < Time::GetMilliseconds())
	//		{
	//			NotifyPlayers(&player, "Message", "You took too long. Passing.",
	//				String("%s took too long and was skipped.", p->mName.GetBuffer()));
	//			AdvanceGame(true);
	//		}
	//	}
	//}

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
	bool victoryCheck = false;

	if (root.mTag == "Name")
	{
		player.mName = root.mValue.AsString();

		if (player.mName.IsValid())
		{
			mSockets.AddUnique(player.mSocket);
			printf("%s has joined the game.\n", player.mName.GetBuffer());
			SendPlayerList();

			// The game is not currently active
			if (!mActive)
			{
				mActive = false;
				mTable.Release();
				SendTable(&player);
				NotifyPlayers(&player, "Play", "You can start the game by pressing Play when everyone is connected.",
					String("%s has joined the game.", player.mName.GetBuffer()));
			}
		}
		return true;
	}
	else if (root.mTag == "Message")
	{
		String text;
		text << "[99FF00]";
		text << player.mName;
		text << "[-]: ";
		text << root.mValue.AsString();

		BeginSend();
		mRoot.mTag = "Message";
		mRoot.mValue = text;
		EndSend(0);
		return true;
	}
	else if (root.mTag == "Discard")
	{
		// Ignore players if it's not their turn, just to be safe
		if (!root.mValue.IsUShortArray()) return false;
		const Array<ushort>& arr = root.mValue.AsUShortArray();
		Card card;

		// Discard the player's cards
		FOREACH(i, arr)
		{
			card = arr[i];
			player.mHand.Remove(card);
			SendMessage(0, String("%s discarded %s %s", player.mName.GetBuffer(),
				(card.value == 11 ? "an" : "a"), g_description[card.value]));
		}
		victoryCheck = true;
	}
	else if (root.mTag == "Play" && mPlayers.GetSize() > 1)
	{
		if (mActive)
		{
			// Ignore players if it's not their turn, just to be safe
			if (&player != mCurrent) return true;
			if (!root.mValue.IsUShortArray()) return false;

			Array<ushort>& arr = root.mValue.ToUShortArray();
			arr.Sort();

			// Play this move
			if (!Play(arr, root))
			{
				SendMessage(&player, "[FF0000]You can't play these cards.");
				return true;
			}
			victoryCheck = true;
		}
		else
		{
			StartGame();
		}
	}

	// Check to see if the player has gone out
	if (victoryCheck && player.mHand.IsEmpty())
	{
		mWinners.Expand() = &player;

		if (mWinners.GetSize() == 1)
		{
			SendMessage(0, String("[99FF00]%s is the president!", player.mName.GetBuffer()));
		}
		else if (mWinners.GetSize() == mPlayers.GetSize())
		{
			SendMessage(0, String("[FF0000]%s is the asshole!", player.mName.GetBuffer()));
		}
		else
		{
			SendMessage(0, String("[FFFF00]%s is out!", player.mName.GetBuffer()));
		}

		// Advance the game
		if (mCurrent == &player) AdvanceGame(true);
	}
	SendPlayerList();
	return true;
}

//============================================================================================================
// Starts a new game
//============================================================================================================

void Server::StartGame()
{
	mWinners.Clear();
	mTable.Release();
	mMove.Clear();
	mActive = true;
	mCurrent = 0;

	Time::Update();
	mRand.SetSeed((uint)Time::GetMilliseconds());

	// Don't accept any more connections
	Array<uint> boot;

	// Find all players who don't have a name
	FOREACH(i, mPlayers)
	{
		Player* p = mPlayers[i];
		if (p->mName.IsEmpty()) boot.Expand() = p->mSocket;
	}

	// Disconnect all players who still don't have a name
	FOREACH(i, boot) mNet.Close(boot[i]);
	mDeck.Clear();
	mDeckCount = (mPlayers.GetSize() > 6) ? 2 : 1;

	// Create the cards
	for (uint a = 0; a < mDeckCount; ++a)
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

	// Clear the players' cards
	FOREACH(i, mPlayers) mPlayers[i]->mHand.Clear();

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

	// Send cards to players
	FOREACH(i, mPlayers)
	{
		Player* player = mPlayers[i];
		SendCards(player);
		SendTable(player);
	}

	// Notify the players of what's going on
	if (mCurrent != 0)
	{
		SendPlayerList();
		NotifyPlayers(mCurrent, "Play", "[FF9966]You[-] have the 3 of clubs and get to start.",
			String("[FF9966]%s[-] has the 3 of clubs and gets to start.", mCurrent->mName.GetBuffer()));
	}
}

//============================================================================================================
// Send out a notification to all players
//============================================================================================================

void Server::NotifyPlayers (Player* target, const String& tag, const String& targetMessage, const String& othersMessage)
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

//============================================================================================================
// Play the specified hand
//============================================================================================================

uint Server::Play (const Array<ushort>& cards, const TreeNode& table)
{
	String msg (mCurrent->mName);

	if (cards.IsEmpty())
	{
		msg << " has passed";
		SendMessage(0, msg.GetBuffer());
		AdvanceGame(true);
		return true;
	}

	// Create a move out of the passed cards
	Move move;
	move.mPlayer = mCurrent;
	move.mCard	 = cards.Front();
	move.mCount  = cards.GetSize();

	bool straight = false;

	// Ensure that the played cards are the same value
	{
		Card card;
		bool sameValue = true;

		FOREACH(i, cards)
		{
			card = cards[i];

			if (move.mCard.value != card.value)
			{
				sameValue = false;
				break;
			}
		}

		// Only straights are allowed -- 5 cards in a row
		if (!sameValue)
		{
			if (cards.GetSize() == 5)
			{
				Card prev;

				for (uint i = 1; i < 5; ++i)
				{
					prev = cards[i-1];
					card = cards[i];

					if (prev.value + 1 != card.value) return false;
				}
			}
			else return false;

			// Played a straight
			straight = true;
		}
	}

	if (move.mCard.value == WILDCARD)
	{
		// A single wildcard beats everything and clears the table
		msg << "[FFFF00] decided to put an end to this BS with a wildcard!";

		Card card;

		FOREACH(i, cards)
		{
			card = cards[i];
			mCurrent->mHand.Remove(card);
		}

		mMoveCount = 0;
		mMove.Clear();
		mTable.Release();
		SendTable(0);
		NotifyPlayers(mCurrent, "Play", msg, msg);
		SendPlayerList();
		return true;
	}
	else if (mMove.IsEmpty())
	{
		mMove.Expand() = move;
		msg << " has started with ";
		++mMoveCount;
	}
	else
	{
		const Move& last = mMove.Back();

		// The played card(s) must be of higher value than the last
		if (last.mCard.value >= move.mCard.value) return false;

		// Power card -- two
		if (move.mCard.value == TWOCARD)
		{
			// A single two can beat up to a pair.
			// A pair of twos can beat a triple, etc.
			if ((last.mCount / 2 + last.mCount % 2) > move.mCount) return false;
			msg << " slammed the table with ";
			mMove.Expand() = move;
			mMoveCount = 0;
		}
		else if (last.mCount != move.mCount)
		{
			// The number of cards must be equal to the previous move
			return false;
		}
		else
		{
			mMove.Expand() = move;
			msg << " has played ";
			++mMoveCount;
		}
	}

	Card card;

	FOREACH(i, cards)
	{
		card = cards[i];
		mCurrent->mHand.Remove(card);
	}

	if (move.mCount == 1)
	{
		msg << ((move.mCard.value == 11 || move.mCard.value == 5) ? "[00FF99]an " : "[00FF99]a ");
		msg << g_description[move.mCard.value];
	}
	else if (move.mCount == 2)
	{
		msg << "[00FF99]a pair of ";
		msg << g_description[move.mCard.value];
		msg << "s";
	}
	else if (straight)
	{
		msg << "[00FF99]a straight beginning at ";
		msg << ((move.mCard.value == 11 || move.mCard.value == 5) ? "[00FF99]an " : "[00FF99]a ");
		msg << g_description[move.mCard.value];
		msg << "!";
	}
	else
	{
		msg << "[00FF99]";
		msg << move.mCount;
		msg << " ";
		msg << g_description[move.mCard.value];
		msg << "s";
	}
	SendMessage(0, msg.GetBuffer());
	mTable = table;
	SendTable(0);

	// See if a discard is possible
	if (mMove.GetSize() >= 3 && mMoveCount >= 3)
	{
		// 3 cards in a row == discard
		if (mMove.Back(1).mCard.value + 1 == mMove.Back(0).mCard.value &&
			mMove.Back(2).mCard.value + 2 == mMove.Back(0).mCard.value)
		{
			uint count = mMove.Back().mCount;
			if (straight) count = 3;
			SendMessage(0, String("[FFFF00]A streak of 3! Discard %u!", count));

			Array<Player*> discards;
			discards.AddUnique(mMove.Back(0).mPlayer);
			discards.AddUnique(mMove.Back(1).mPlayer);
			discards.AddUnique(mMove.Back(2).mPlayer);

			FOREACH(i, discards)
			{
				Player* dis = discards[i];

				if (mPlayers.Contains(dis) && dis->mHand.IsValid())
				{
					uint val = (count < dis->mHand.GetSize() ? count : dis->mHand.GetSize());
					BeginSend();
					mRoot.mTag = "Discard";
					mRoot.mValue = val;
					EndSend(dis->mSocket);
				}
			}

			// Reset the counter
			mMoveCount = 0;
		}
	}

	AdvanceGame(false);
	return 1;
}

//============================================================================================================
// Advance the game to the next player
//============================================================================================================

void Server::AdvanceGame (bool passed)
{
	bool found = false;
	Player* next (0);

	uint count = 0;

	FOREACH(i, mPlayers)
	{
		Player* player = mPlayers[i];

		if (player->mHand.IsValid())
		{
			++count;
		}
	}

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

	if (next == 0 || next == mCurrent || count < 2)
	{
		SendMessage(0, "Game over!");
		StartGame();
	}
	else
	{
		mCurrent = next;

		if (passed && mMove.IsValid())
		{
			const Move& last = mMove.Back();

			if (last.mPlayer == mCurrent || !mPlayers.Contains(last.mPlayer) ||
				last.mPlayer == 0 || last.mPlayer->mHand.IsEmpty())
			{
				SendMessage(0, "[6699FF]The cards have come a full circle. New round.");
				mMove.Release();
				mTable.Release();
				mMoveCount = 0;
				SendTable(0);
			}
		}

		SendPlayerList();
		SendTable(0);
		NotifyPlayers(mCurrent, "Play", "[6699FF]Your turn!", String("[6699FF]%s[-]'s turn.",
			mCurrent->mName.GetBuffer()));
	}
}

//============================================================================================================
// Send the list of players to everyone
//============================================================================================================

void Server::SendPlayerList()
{
	BeginSend();
	{
		mRoot.mTag = "Players";
		String out;

		FOREACH(i, mPlayers)
		{
			const Player* p = mPlayers[i];

			if (p->mName.IsValid())
			{
				if (mWinners.IsValid() && p == mWinners.Front())
				{
					out = "[99FF66]";
					out << p->mName;
				}
				else if (p->mHand.IsEmpty())
				{
					out = "[777777]";
					out << p->mName;
				}
				else if (p == mCurrent)
				{
					out = "[6699FF]";
					out << p->mName;
					out << " (";
					out << p->mHand.GetSize();
					out << ")";
				}
				else
				{
					out = p->mName;
					out << " [777777](";
					out << p->mHand.GetSize();
					out << ")";
				}
				mRoot.AddChild("Name", out);
			}
		}
	}
	EndSend(0);
}

//============================================================================================================
// Send the player's hand to the specified player
//============================================================================================================

void Server::SendCards (Player* player)
{
	if (player != 0)
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
}

//============================================================================================================
// Send the entire table's contents to the specified player
//============================================================================================================

void Server::SendTable (Player* player)
{
	BeginSend();
	mRoot = mTable;
	mRoot.mTag = "Table";
	EndSend(player != 0 ? player->mSocket : 0);
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