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
			ServerMessage	= 0,
			EntityUpdate	= 1,
			Pathfinding		= 2,
			PlayerEnter		= 3,
			PlayerExit		= 4,
			PlayerMove		= 5,
			PlayerRun		= 6,
		};
	};

	struct Player
	{
		uint		mId;
		uint		mZoneID;
		String		mName;
		String		mZone;
		Vector3f	mPos;
		Quaternion	mRot;
		Memory		mMem;
		bool		mNewData;

		Player() : mId(0), mZoneID(0), mNewData(false) {}

		inline void Lock()   const { mMem.Lock();   }
		inline void Unlock() const { mMem.Unlock(); }
		inline void Clear() { mMem.Clear(); mNewData = false; }
		inline void Append (const byte* buffer, uint size) { mMem.Append(buffer, size); mNewData = true; }
	};

	Network mNet;
	bool mNewData;

	PointerArray<Player> mUsed;
	PointerArray<Player> mUnused;

public:

	TestApp() : mNewData(false) {}

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

	void SendMessage (const String& msg, const Network::Address& a, uint id);

	void SendRaw (uint zoneID, const byte* buffer, uint size, Player* exclude = 0);

	void Send (uint zoneID, const Memory& mem, Player* exclude = 0);

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

	mUsed.Lock();
	{
		Player* player = mUnused.IsValid() ? mUnused.Shrink(false) : new Player();
		player->mId = id;
		mUsed.Expand() = player;
		userData = player;
	}
	mUsed.Unlock();

	SendMessage("Hello from the server!", a, id);
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
	//printf("[%u] Received %u bytes from %s\n", Thread::GetID(), bufferSize, a.ToString().GetBuffer());

	if (userData != 0)
	{
		mNewData = true;

		Player* player = (Player*)userData;

		player->Lock();
		player->Append(buffer, bufferSize);
		player->Unlock();
	}
}

//============================================================================================================
// A connection has been closed -- clear its memory buffer and move it to the 'unused' section
//============================================================================================================

void TestApp::OnClose (const Network::Address& a, uint id, VoidPtr& userData)
{
	if (a.mIp != 0)
	{
		printf("[%u] Disconnected from %s\n", Thread::GetID(), a.ToString().GetBuffer());
	}
	else
	{
		printf("[%u] Listening port %u has been closed\n", id, a.mPort);
	}

	if (userData != 0)
	{
		Player* player = (Player*)userData;

		player->Lock();
		{
			// Player exit notification
			Memory mem;
			mem.Append(PacketID::PlayerExit);
			mem.Append(player->mId);
			Send(player->mZoneID, mem, player);

			player->Clear();
		}
		player->Unlock();

		mUsed.Lock();
		{
			mUsed.Remove(player);
			mUnused.Expand() = player;
		}
		mUsed.Unlock();
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

void TestApp::SendMessage (const String& msg, const Network::Address& a, uint id)
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

void TestApp::SendRaw (uint zoneID, const byte* buffer, uint size, Player* exclude)
{
	for (uint i = mUsed.GetSize(); i > 0; )
	{
		const Player* player = mUsed[--i];

		if (player != exclude && player->mZoneID == zoneID)
		{
			mNet.Send(buffer, size, player->mId);
		}
	}
}

//============================================================================================================
// Send the specified data to everyone in the same area, excluding the target
//============================================================================================================

void TestApp::Send (uint zoneID, const Memory& mem, Player* exclude)
{
	uint size = mem.GetSize();

	for (uint i = mUsed.GetSize(); i > 0; )
	{
		const Player* player = mUsed[--i];

		if (player != exclude && player->mZoneID == zoneID)
		{
			mNet.Send(&size, 4, player->mId);
			mNet.Send(mem.GetBuffer(), size, player->mId);
		}
	}
}

//============================================================================================================
// Process all waiting data
//============================================================================================================

void TestApp::Process()
{
	mUsed.Lock();
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
					printf("Expecting %u bytes of %u available\n", packetLength, size);
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
	mUsed.Unlock();
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
		if (packetId == PacketID::EntityUpdate)
		{
			String name;
			//String parent;
			//Vector3f pos;
			//Quaternion rot;

			Memory::Extract(buffer, size, name);
			//Memory::Extract(buffer, size, pos);
			//Memory::Extract(buffer, size, rot);
			//Memory::Extract(buffer, size, parent);

			//printf("Received entity update (%d bytes)\n", originalSize);
			//printf("- Name: %s\n", name.GetBuffer());
			//printf("- Pos: %.3f %.3f %.3f\n", pos.x, pos.y, pos.z);
			//printf("- Rot: %.3f %.3f %.3f %.3f\n", rot.x, rot.y, rot.z, rot.w);
			//printf("- Parent: %s\n", parent.GetBuffer());

			printf("- %s sent an entity update: %s\n", player->mName.GetBuffer(), name.GetBuffer());

			// Forward the packet to everyone in the same area
			SendRaw(player->mZoneID, originalBuffer - 4, originalSize + 4, player);
		}
		else if (packetId == PacketID::PlayerExit)
		{
			String zone;
			Memory::Extract(buffer, size, zone);
			printf("- %s has left %s heading to %s\n", player->mName.GetBuffer(),
				player->mZone.GetBuffer(), zone.GetBuffer());

			// Player exit notification
			Memory mem;
			mem.Append(PacketID::PlayerExit);
			mem.Append(player->mId);
			Send(player->mZoneID, mem, player);

			// The player is now in the void
			player->mZoneID = 0;
		}
		else if (packetId == PacketID::PlayerEnter)
		{
			Memory::Extract(buffer, size, player->mName);
			Memory::Extract(buffer, size, player->mPos);
			Memory::Extract(buffer, size, player->mRot);
			Memory::Extract(buffer, size, player->mZone);

			printf("- %s has entered %s\n", player->mName.GetBuffer(), player->mZone.GetBuffer());

			// Update the local values
			player->mZoneID = HashKey(player->mZone);

			// Send out a similar notification to others
			Memory mem;
			mem.Append(PacketID::PlayerEnter);
			mem.Append(player->mId);
			mem.Append(player->mName);
			mem.Append(player->mPos);
			mem.Append(player->mRot);
			Send(player->mZoneID, mem, player);
		}
		else if (packetId == PacketID::Pathfinding)
		{
			// Forward the packet to everyone in the same area
			SendRaw(player->mZoneID, originalBuffer - 4, originalSize + 4, player);

			printf("- %s sent a pathfinding update\n", player->mName.GetBuffer());
		}
		else if (packetId == PacketID::PlayerMove)
		{
			Vector3f pos;
			Memory::Extract(buffer, size, pos);

			Memory mem;
			mem.Append(PacketID::PlayerMove);
			mem.Append(player->mId);
			mem.Append(pos);

			// Forward the packet to everyone in the same area
			Send(player->mZoneID, mem, player);
			
			printf("- %s is now moving to (%f %f %f)\n", player->mName.GetBuffer(), pos.x, pos.y, pos.z);
		}
		else if (packetId == PacketID::PlayerRun)
		{
			bool val;
			Memory::Extract(buffer, size, val);

			Memory mem;
			mem.Append(PacketID::PlayerMove);
			mem.Append(player->mId);
			mem.Append(val);

			// Forward the packet to everyone in the same area
			Send(player->mZoneID, mem, player);
			
			printf("- %s is now %s\n", player->mName.GetBuffer(), val ? "running" : "walking");
		}
		else
		{
			printf("- Received unknown ID: %d (%x %x %x %x)\n", packetId,
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