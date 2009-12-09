#include "../../../Engine/Network/Include/_All.h"
#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
private:

	struct PacketID
	{
		enum
		{
			KeepAlive		= 0,
			ServerMessage	= 1,
			PlayerMove		= 2,
			PlayerRun		= 3,
			PlayerEnter		= 4,
			PlayerExit		= 5,
			EntityUpdate	= 6,
			SayMessage		= 7,
		};
	};

	struct Entity
	{
		String		mName;
		Vector3f	mPos;
		Quaternion	mRot;
		String		mParent;
	};

	struct Player;
	struct Zone
	{
		String			mName;
		Array<Player*>	mPlayers;
		Hash<Entity>	mEntities;
	};

	struct Player
	{
		typedef Network::Address Address;

		ulong		mTime;
		uint		mId;
		Address		mAddr;
		String		mName;
		Vector3f	mPos;
		Vector3f	mDir;
		Zone*		mZone;
		Memory		mMem;
		bool		mNewData;

		Player() { Clear(); }

		inline void Lock()   const { mMem.Lock();   }
		inline void Unlock() const { mMem.Unlock(); }

		inline void Clear()
		{
			mTime	 = 0;
			mId		 = 0;
			mZone	 = 0;
			mNewData = false;

			mName.Clear();
			mMem.Clear();
		}

		inline void Append (const byte* buffer, uint size)
		{
			mMem.Append(buffer, size);
			mNewData = true;
		}
	};

private:

	Network mNet;
	bool	mNewData;
	ulong	mTime;
	ulong	mLastCheck;

	PointerArray<Player> mUsed;
	PointerArray<Player> mUnused;

	Hash<Zone>	mZones;

public:

	TestApp() : mNewData(false), mTime(0), mLastCheck(0) {}

	inline void Lock()   const { mUsed.Lock();   }
	inline void Unlock() const { mUsed.Unlock(); }

private:

	void OnConnect (const Network::Address& a, uint id, VoidPtr& userData, const String& original);

	void OnReceive (const Network::Address& a,
					uint					id,
					VoidPtr&				userData,
					const byte*				buffer,
					uint					bufferSize,
					Thread::IDType			threadId);

	void OnClose (const Network::Address& a, uint id, VoidPtr& userData);

	void OnError (const Network::Address&	a,
				  uint						id,
				  VoidPtr&					userData,
				  const char*				buffer);

private:

	void SendMessage (const String& msg, uint id, const Network::Address& a = Network::Address());

	void SendRaw (Zone* zone, const byte* buffer, uint size, Player* exclude = 0);

	void Send (Zone* zone, const Memory& mem, Player* exclude = 0);

	void Process();

	void Process (Player* player, const byte* buffer, uint size);

public:

	void Run()
	{
		mNet.SetOnConnect( bind(&TestApp::OnConnect, this) );
		mNet.SetOnReceive( bind(&TestApp::OnReceive, this) );

		mNet.SetOnClose( bind(&TestApp::OnClose, this) );
		mNet.SetOnError( bind(&TestApp::OnError, this) );

		mNet.Listen(5112);
		mNet.SpawnWorkerThread();

		for (;;)
		{
			Time::Update();
			mTime = Time::GetMilliseconds();

			// Check for idle connections once per second
			if (mLastCheck + 1000 < mTime)
			{
				mLastCheck = mTime;

				for (uint i = mUsed.GetSize(); i > 0; )
				{
					Player* player = mUsed[--i];

					// If no message has come from the player for 10 seconds, disconnect him
					if (player->mTime + 30000 < mTime)
					{
						player->Lock();
						{
							printf("%s has been idle for 10 seconds. Disconnecting.\n", player->mName.GetBuffer());
							SendMessage("Your networking code is out of date. Please update your TailTowns \
project before connecting to the SyncServer again.", player->mId);
							mNet.Close(player->mId);
							player->Clear();

							Lock();
							mUsed.Remove(player);
							mUnused.Expand() = player;
							Unlock();
						}
						player->Unlock();
					}
				}
			}

			// If we have new data, process it. Otherwise go to sleep for a bit.
			if (mNewData)
			{
				mNewData = false;
				Process();
				Thread::Sleep(0);
			}
			else
			{
				Thread::Sleep(1);
			}
		}
	}
};

//============================================================================================================
// New connection has been established -- Create a new memory buffer for it
//============================================================================================================

void TestApp::OnConnect (const Network::Address& a, uint id, VoidPtr& userData, const String& original)
{
	printf("[%u] Connected to %s\n", Thread::GetID(), original.GetBuffer());

	Lock();
	{
		Player* player	= mUnused.IsValid() ? mUnused.Shrink(false) : new Player();
		player->mId		= id;
		player->mAddr	= a;
		player->mTime	= mTime;
		mUsed.Expand()	= player;
		userData		= player;
	}
	Unlock();

	SendMessage("Hello from the server!", id, a);
}

//============================================================================================================
// New data has been received -- use the memory buffer we assigned in OnConnect, appending to it
//============================================================================================================

void TestApp::OnReceive (const Network::Address&	a,
						 uint						id,
						 VoidPtr&					userData,
						 const byte*				buffer,
						 uint						bufferSize,
						 Thread::IDType				threadId)
{
	if (userData != 0)
	{
		mNewData = true;

		Player* player = (Player*)userData;

		player->Lock();
		player->Append(buffer, bufferSize);
		player->mTime = mTime;
		player->Unlock();
	}
}

//============================================================================================================
// A connection has been closed -- clear its memory buffer and move it to the 'unused' section
//============================================================================================================

void TestApp::OnClose (const Network::Address& a, uint id, VoidPtr& userData)
{
	Player* player = (Player*)userData;

	if (player != 0 && player->mId)
	{
		printf("%s has disconnected\n", player->mName.GetBuffer());

		Lock();
		{
			if (player->mZone != 0)
			{
				// Player exit notification
				Memory mem;
				mem.Append(PacketID::PlayerExit);
				mem.Append(player->mId);
				Send(player->mZone, mem, player);

				// Remove this player from the zone
				player->mZone->mPlayers.Remove(player);
			}

			// Move this player to the 'unused' list
			mUsed.Remove(player);
			player->Clear();
			mUnused.Expand() = player;
		}
		Unlock();
	}
}

//============================================================================================================
// An error has occured
//============================================================================================================

void TestApp::OnError ( const Network::Address&	a,
						uint					id,
						VoidPtr&				userData,
						const char*				buffer )
{
	printf("[%u] Error occured while communicating with %s (%s)\n",
		Thread::GetID(), a.ToString().GetBuffer(), buffer);
}

//============================================================================================================
// Sends a server message to the specified client
//============================================================================================================

void TestApp::SendMessage (const String& msg, uint id, const Network::Address& a)
{
	Memory mem;
	mem.Append(0);
	mem.Append(PacketID::ServerMessage);
	mem.Append(msg);
	
	// Update the size of the buffer
	*(uint32*)mem.GetBuffer() = (uint32)mem.GetSize() - 4;

	// Send the complete packet
	mNet.Send(mem.GetBuffer(), mem.GetSize(), id, a);
}

//============================================================================================================
// Send the specified data to everyone in the same area, excluding the target
//============================================================================================================

void TestApp::SendRaw (Zone* zone, const byte* buffer, uint size, Player* exclude)
{
	if (zone != 0)
	{
		for (uint i = zone->mPlayers.GetSize(); i > 0; )
		{
			const Player* player = zone->mPlayers[--i];

			if (player != exclude)
			{
				mNet.Send(buffer, size, player->mId);
			}
		}
	}
}

//============================================================================================================
// Send the specified data to everyone in the same area, excluding the target
//============================================================================================================

void TestApp::Send (Zone* zone, const Memory& mem, Player* exclude)
{
	if (zone != 0)
	{
		uint size = mem.GetSize();

		for (uint i = zone->mPlayers.GetSize(); i > 0; )
		{
			const Player* player = zone->mPlayers[--i];

			if (player != exclude)
			{
				mNet.Send(&size, 4, player->mId);
				mNet.Send(mem.GetBuffer(), size, player->mId);
			}
		}
	}
}

//============================================================================================================
// Process all waiting data
//============================================================================================================

void TestApp::Process()
{
	Lock();
	{
		for (uint i = mUsed.GetSize(); i > 0; )
		{
			Player* player = mUsed[--i];
			if (!player->mNewData) continue;

			// Get the current memory buffer
			uint packetLength;
			Memory& mem = player->mMem;
			const byte* buffer = mem.GetBuffer();
			const byte* nextPacket = buffer;
			uint size = mem.GetSize();

			// Extract the packet length
			while (Memory::Extract(buffer, size, packetLength))
			{
				if (packetLength > size)
				{
					//printf("Expecting %u bytes of %u available\n", packetLength, size);
					break;
				}
				else
				{
					// Process the data we just received
					Process(player, buffer, packetLength);

					// Skip past this packet
					buffer += packetLength;
					size   -= packetLength;

					// This is now the starting position for the next packet
					nextPacket = buffer;
				}
			}

			// If we've processed some data, remove it from memory
			if (nextPacket > mem.GetBuffer())
			{
				mem.Remove(nextPacket - mem.GetBuffer());
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Processes the specified chunk of data
//============================================================================================================

void TestApp::Process (Player* player, const byte* buffer, uint size)
{
	uint32 packetId;
	const byte* originalBuffer = buffer;
	uint32 originalSize = (uint32)size;

	if (Memory::Extract(buffer, size, packetId))
	{
		if (packetId == PacketID::KeepAlive)
		{
			// Do nothing. This message is used to keep the connection alive.
		}
		else if (packetId == PacketID::PlayerMove)
		{
			Vector3f pos;
			Memory::Extract(buffer, size, pos);

			if (pos.GetDistanceTo(player->mPos) > 0.1f)
			{
				player->mDir = Normalize(pos - player->mPos);
				player->mPos = pos;

				Memory mem;
				mem.Append(PacketID::PlayerMove);
				mem.Append(player->mId);
				mem.Append(pos);

				// Forward the packet to everyone in the same area
				Send(player->mZone, mem, player);
				
				printf("- %s is now moving to (%f %f %f)\n", player->mName.GetBuffer(), pos.x, pos.y, pos.z);
			}
		}
		else if (packetId == PacketID::PlayerRun)
		{
			bool val;
			Memory::Extract(buffer, size, val);

			Memory mem;
			mem.Append(PacketID::PlayerRun);
			mem.Append(player->mId);
			mem.Append(val);

			// Forward the packet to everyone in the same area
			Send(player->mZone, mem, player);
			
			printf("- %s is now %s\n", player->mName.GetBuffer(), val ? "running" : "walking");
		}
		else if (packetId == PacketID::PlayerEnter)
		{
			String zone;

			Memory::Extract(buffer, size, player->mName);
			Memory::Extract(buffer, size, player->mPos);
			Memory::Extract(buffer, size, player->mDir);
			Memory::Extract(buffer, size, zone);

			if (player->mName.BeginsWith("Scenes/") || player->mName.IsEmpty())
			{
				player->Lock();
				uint id = player->mId;
				player->Clear();

				printf("%s is out of date. Disconnecting. (%s)\n", player->mName.GetBuffer(),
					player->mAddr.ToString().GetBuffer());

				SendMessage("Your networking code is out of date. Please update your TailTowns \
project before connecting to the SyncServer again.", id);
				mNet.Close(id);

				mUsed.Remove(player);
				mUnused.Expand() = player;
				player->Unlock();
				return;
			}

			printf("%s has entered %s\n", player->mName.GetBuffer(), zone.GetBuffer());

			// Update the player's zone
			player->mZone = &mZones[zone];

			// Remember the zone's name
			if (player->mZone->mName.IsEmpty())
				player->mZone->mName = zone;

			// Safety check
			ASSERT(player->mZone->mName == zone, "Hash key collision detected");

			// Add this player to the zone
			player->mZone->mPlayers.AddUnique(player);

			// Send out a similar notification to others
			Memory mem;
			mem.Append(PacketID::PlayerEnter);
			mem.Append(player->mId);
			mem.Append(player->mName);
			mem.Append(player->mPos);
			mem.Append(player->mDir);
			Send(player->mZone, mem, player);

			// Run through all players in the same room and send their information to the new player
			for (uint i = player->mZone->mPlayers.GetSize(); i > 0; )
			{
				const Player* pe = player->mZone->mPlayers[--i];

				if (pe != player)
				{
					mem.Clear();
					uint32* size = mem.Append((uint)0);
					mem.Append(PacketID::PlayerEnter);
					mem.Append(pe->mId);
					mem.Append(pe->mName);
					mem.Append(pe->mPos);
					mem.Append(pe->mDir);
					*size = (uint32)(mem.GetSize() - 4);
					mNet.Send(mem.GetBuffer(), mem.GetSize(), player->mId);
				}
			}

			// Run through all updated entities in the room and send their information to the new player
			const Array<Entity>& entities = player->mZone->mEntities.GetAllValues();

			for (uint i = entities.GetSize(); i > 0; )
			{
				const Entity& ent = entities[--i];

				mem.Clear();
				uint32* size = mem.Append((uint)0);
				mem.Append(PacketID::EntityUpdate);
				mem.Append(ent.mName);
				mem.Append(ent.mPos);
				mem.Append(ent.mRot);
				mem.Append(ent.mParent);
				*size = (uint32)(mem.GetSize() - 4);
				mNet.Send(mem.GetBuffer(), mem.GetSize(), player->mId);
			}
		}
		else if (packetId == PacketID::PlayerExit)
		{
			if (player->mZone != 0)
			{
				String zone;
				Memory::Extract(buffer, size, zone);
				printf("%s has left %s heading to %s\n", player->mName.GetBuffer(),
					(player->mZone == 0) ? "<limbo>" : player->mZone->mName.GetBuffer(), zone.GetBuffer());

				// Player exit notification
				Memory mem;
				mem.Append(PacketID::PlayerExit);
				mem.Append(player->mId);
				Send(player->mZone, mem, player);

				// The player is now in the void
				player->mZone->mPlayers.Remove(player);
				player->mZone = 0;
			}
		}
		else if (packetId == PacketID::EntityUpdate)
		{
			String name;
			String parent;
			Vector3f pos;
			Quaternion rot;

			Memory::Extract(buffer, size, name);
			Memory::Extract(buffer, size, pos);
			Memory::Extract(buffer, size, rot);
			Memory::Extract(buffer, size, parent);

			// Remember this entity's information
			if (player->mZone != 0)
			{
				Entity& ent = player->mZone->mEntities[name];
				ent.mName	= name;
				ent.mPos	= pos;
				ent.mRot	= rot;
				ent.mParent	= parent;
			}

			printf("- %s sent an entity update: %s\n", player->mName.GetBuffer(), name.GetBuffer());

			// Forward the packet to everyone in the same area
			SendRaw(player->mZone, originalBuffer - 4, originalSize + 4, player);
		}
		else if (packetId == PacketID::SayMessage)
		{
			String message;
			Memory::Extract(buffer, size, message);

			Memory mem;
			mem.Append(PacketID::SayMessage);
			mem.Append(player->mId);
			mem.Append(message);

			Send(player->mZone, mem, player);

			printf("- %s says: %s\n", player->mName.GetBuffer(), message.GetBuffer());
		}
		else
		{
			printf("- Received unknown ID from %s: %d (%x %x %x %x)\n", player->mName.GetBuffer(),
				 packetId,
				(packetId >> 24) & 0xFF,
				(packetId >> 16) & 0xFF,
				(packetId >> 8) & 0xFF,
				 packetId & 0xFF);
		}
	}
}

//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");
	TestApp app;
	app.Run();
	return 0;
}