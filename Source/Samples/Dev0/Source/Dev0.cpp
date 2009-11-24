#include "../../../Engine/Network/Include/_All.h"
#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================

class TestApp
{
private:

	struct InBuffer
	{
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

	void Process (const byte* buffer, uint size);

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
		InBuffer* ib = mUnused.IsValid() ? mUnused.Shrink(false) : new InBuffer();
		mUsed.Expand() = ib;
		userData = ib;

		String str ("Hi there!");
		uint length = str.GetLength();

		mNet.Send(&length, 4, id, a);
		mNet.Send(str.GetBuffer(), length, id, a);
	}
	mUsed.Unlock();
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
					Process(buffer, packetLength);

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

void TestApp::Process (const byte* buffer, uint size)
{
	TreeNode tree;
					
	if (tree.SerializeFrom(buffer, size))
	{
		String s;
		tree.SerializeTo(s, 1);
		printf("[%u] Received:\n%s\n", Thread::GetID(), s.GetBuffer());

		String str ("Received OK");
		uint length = str.GetLength();

		mNet.Send(&length, 4);
		mNet.Send(str.GetBuffer(), length);
	}
	else
	{
		printf("[%u] ERROR: SerializeFrom failed\n", Thread::GetID());
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