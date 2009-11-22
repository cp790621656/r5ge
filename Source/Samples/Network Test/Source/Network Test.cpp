#include "../../../Engine/Network/Include/_All.h"
#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================

uint				g_lastID = 0;
Network::Address	g_lastAddr;
Network				net;

//============================================================================================================

void OnConnect (const Network::Address& a, uint id, VoidPtr& userData, const String& original)
{
	g_lastAddr = a;
	g_lastID = id;
	printf("[%u] Connected to %s\n", Thread::GetID(), original.GetBuffer());
}

//============================================================================================================

void OnClose (const Network::Address& a, uint id, VoidPtr& userData)
{
	g_lastAddr.mIp		= 0;
	g_lastAddr.mPort	= 0;
	g_lastID			= 0;

	if (a.mIp != 0)
	{
		printf("[%u] Disconnected from %s\n", Thread::GetID(), a.ToString().GetBuffer());
	}
	else
	{
		printf("[%u] Listening port %u has been closed\n", id, a.mPort);
	}
}

//============================================================================================================

void OnReceive (const Network::Address& a,
				uint					id,
				VoidPtr&				userData,
				const byte*				buffer,
				uint					bufferSize,
				Thread::IDType			threadId)
{
	g_lastAddr = a;
	g_lastID = id;
	printf("[%u] Received %u bytes from %s\n", Thread::GetID(), bufferSize, a.ToString().GetBuffer());
}

//============================================================================================================

void OnError (const Network::Address&	a,
			  uint						id,
			  VoidPtr&					userData,
			  const char*				buffer)
{
	printf("[%u] Error occured while communicating with %s (%s)\n", Thread::GetID(),
		a.ToString().GetBuffer(), buffer);
}

//============================================================================================================

void OnLog (const String& text)
{
	printf("%s\n", text.GetBuffer());
}

//============================================================================================================

bool Send (const TreeNode& root, uint count = 1)
{
	Memory mem;
	uint32* size = (uint32*)mem.Expand(4);
	root.SerializeTo(mem);
	*size = (uint32)(mem.GetSize() - 4);

	if (*size > 0 && count > 0)
	{
		for (uint i = 0; i < count; ++i)
		{
			if (!net.Send(mem.GetBuffer(), mem.GetSize(), g_lastID, g_lastAddr))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

//============================================================================================================

int main (int argc, char* argv[])
{
	{
		net.SetOnConnect (&OnConnect);
		net.SetOnReceive (&OnReceive);
		net.SetOnClose	 (&OnClose);
		net.SetOnError	 (&OnError);
		net.SetOnLog	 (&OnLog);

		while (true)
		{
			printf("Command (%u):\n", g_lastID & 0xFFFF);
			int c = getchar(); getchar();

			if		(c == 'z') { net.Close(g_lastID); }
			else if (c == 'c') { net.Connect("127.0.0.1:5112"); }
			else if (c == '7') { g_lastID = net.Listen(1337, Network::Socket::Type::UDP); g_lastAddr = net.Resolve("127.0.0.1:1338"); }
			else if (c == '8') { g_lastID = net.Listen(1338, Network::Socket::Type::UDP); g_lastAddr = net.Resolve("127.0.0.1:1337"); }
			else if (c == '9') { g_lastID = net.Listen(1339, Network::Socket::Type::UDP); g_lastAddr = net.Resolve("127.0.0.1:1337"); }
			else if (c == 'u') net.SpawnWorkerThread();
			else if (c == 'r') g_lastID = 0;
			else if	(c == 'l')
			{
				if ( net.Listen(1050, Network::Socket::Type::TCP) ) puts("Listening to port 1050");
				else puts("Unable to listen to port 1050 -- it is likely already in use");
			}
			else if (c == 'm')
			{
				TreeNode node ("Root");
				node.AddChild("First", 0.123f);
				node.AddChild("Second", Vector3f(1.0f, 2.0f, 3.0f));
				node.AddChild("Third", "Hello world!");

				if (Send(node, 100)) puts("Spam sent...");
				else puts("Spam failed!");
			}
			else if (c == 's')
			{
				TreeNode node ("Root");
				node.AddChild("First", 0.123f);
				node.AddChild("Second", Vector3f(1.0f, 2.0f, 3.0f));
				node.AddChild("Third", "Hello world!");

				if (Send(node)) puts("Sent...");
				else puts("Send failed!");
			}
			else if	(c == 'q') break;
		}
	}
	puts("Finished");
	getchar();
	return 0;
}