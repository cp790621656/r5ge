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
			ZoneSwitch		= 1,
			EntityUpdate	= 2,
			Pathfinding		= 3,
		};
	};

	struct InBuffer
	{
		uint	mId;
		Memory	mMem;
		bool	mNewData;

		InBuffer() : mNewData(false) {}

		inline void Lock()   const { mMem.Lock();   }
		inline void Unlock() const { mMem.Unlock(); }
		inline void Clear() { mMem.Clear(); mNewData = false; }
		inline void Append (const byte* buffer, uint size) { mMem.Append(buffer, size); mNewData = true; }
	};

	Network mNet;
	bool mNewData;

	PointerArray<InBuffer> mUsed;
	PointerArray<InBuffer> mUnused;

public:

	TestApp() : mNewData(false) {}

private:

	void SendMessage (const String& msg, const Network::Address& a, uint id);

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

	void Process();

	void Process (InBuffer* ib, const byte* buffer, uint size);

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
// New connection has been established -- Create a new memory buffer for it
//============================================================================================================

void TestApp::OnConnect (const Network::Address& a, uint id, VoidPtr& userData, const String& original)
{
	printf("[%u] Connected to %s\n", Thread::GetID(), original.GetBuffer());

	mUsed.Lock();
	{
		InBuffer* ib = mUnused.IsValid() ? mUnused.Shrink(false) : new InBuffer();
		ib->mId = id;
		mUsed.Expand() = ib;
		userData = ib;
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
	printf("[%u] Received %u bytes from %s\n", Thread::GetID(), bufferSize, a.ToString().GetBuffer());

	if (userData != 0)
	{
		mNewData = true;

		InBuffer* ib = (InBuffer*)userData;

		ib->Lock();
		ib->Append(buffer, bufferSize);
		ib->Unlock();
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
		InBuffer* ib = (InBuffer*)userData;

		ib->Lock();
		ib->Clear();
		ib->Unlock();

		mUsed.Lock();
		{
			mUsed.Remove(ib);
			mUnused.Expand() = ib;
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
// Process all waiting data
//============================================================================================================

void TestApp::Process()
{
	mUsed.Lock();
	{
		for (uint i = mUsed.GetSize(); i > 0; )
		{
			InBuffer* ib = mUsed[--i];
			if (!ib->mNewData) continue;

			// Get the current memory buffer
			uint packetLength;
			Memory& mem = ib->mMem;
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
					Process(ib, buffer, packetLength);

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

void TestApp::Process (InBuffer* ib, const byte* buffer, uint size)
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

			// Forward the packet to everyone
			mNet.Send(&originalSize, 4);
			mNet.Send(originalBuffer, originalSize);

			printf("User %d sent an entity update: %s\n", ib->mId, name.GetBuffer());
		}
		else if (packetId == PacketID::ZoneSwitch)
		{
			String zone;
			Memory::Extract(buffer, size, zone);
			printf("User %d switched zones: %s\n", ib->mId, zone.GetBuffer());
		}
		else if (packetId == PacketID::Pathfinding)
		{
			mNet.Send(&originalSize, 4);
			mNet.Send(originalBuffer, originalSize);
			printf("User %d sent a pathfinding update\n", ib->mId);
		}
		else
		{
			printf("Received unknown ID: %d (%x %x %x %x)\n", packetId,
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