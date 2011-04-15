#include "../Include/_All.h"

//============================================================================================================

#ifdef _WINDOWS
 #include <winsock2.h>
 #pragma comment(lib, "ws2_32.lib")

 // Error codes
#ifndef EAGAIN
 #define EAGAIN			WSATRY_AGAIN
#endif
 #define EWOULDBLOCK	WSAEWOULDBLOCK
 #define ETIMEDOUT		WSAETIMEDOUT
 #define ECONNRESET		WSAECONNRESET
 #define EINPROGRESS	WSAEINPROGRESS
 #define EALREADY		WSAEALREADY
 #define ECONNABORTED	WSAECONNABORTED
 #define socklen_t		int
#else
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <errno.h>
 #include <netdb.h>
#endif

using namespace R5;

//============================================================================================================
// Helper functions
//============================================================================================================

namespace R5
{
// Shortens the code
typedef Network::Socket Socket;

inline void SockaddrToAddress (Network::Address& a, const sockaddr_in& s)
{
	a.mIp   = htonl(s.sin_addr.s_addr);
	a.mPort = htons(s.sin_port);
}

inline void AddressToSockaddr (const Network::Address& a, sockaddr_in& s)
{
	memset(&s, 0, sizeof(sockaddr_in));
	s.sin_family		= AF_INET;
	s.sin_addr.s_addr	= htonl(a.mIp);
	s.sin_port			= htons(a.mPort);
}

//============================================================================================================
// Closes the specified socket
//============================================================================================================

inline void CloseSocket (const Socket& s)
{
	if (s.mSocket != Socket::Invalid)
	{
#ifdef _WINDOWS
		::closesocket(s.mSocket);
#else
		::close(s.mSocket);
#endif
		(const_cast<Socket&>(s)).Reset();
	}
}

//============================================================================================================
// Creates an appropriate socket
//============================================================================================================

bool CreateSocket (Socket& s, uint type)
{
#ifdef _WINDOWS
	// Create a non-blocking socket
	s.mSocket = (Socket::Identifier)::WSASocket(AF_INET, (type == Network::Socket::Type::TCP) ?
		SOCK_STREAM : SOCK_DGRAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
#else
	// Create a blocking socket
	s.mSocket = (Socket::Identifier)::socket(PF_INET, (type == Network::Socket::Type::TCP) ?
		SOCK_STREAM : SOCK_DGRAM, 0);
#endif

	s.mType = type;
	s.mAction = Socket::Action::Disconnected;
	return (s.mSocket != Socket::Invalid);
}

//============================================================================================================
// Debugging mode should forward all log messages to System::Log
//============================================================================================================

#ifdef _DEBUG
  #define DEBUG_LOG(text)	{ System::Log("[NETWORK] %s", text.GetBuffer()); }
  #define NORMAL_LOG(text)	{ if (mOnLog) mOnLog(text); DEBUG_LOG(text); }
#else
  #define DEBUG_LOG(text)
  #define NORMAL_LOG(text)	{ if (mOnLog) mOnLog(text); }
#endif

//============================================================================================================
// Network::Connect thread, since Network::_Connect() is a blocking call
//============================================================================================================

R5_THREAD_FUNCTION(Connect_Thread, ptr)
{
	((Network*)ptr)->_Connect();
	return 0;
}

//============================================================================================================
// Network::_Update() worker thread
//============================================================================================================

R5_THREAD_FUNCTION(Update_Thread, ptr)
{
	((Network*)ptr)->_Update();
	return 0;
}
} // namespace R5

//============================================================================================================
// INTERNAL: Retrieves the code of the last error
//============================================================================================================

int Network::_GetError()
{
#ifdef _WINDOWS
	return ::WSAGetLastError();
#else
	return errno;
#endif
}

//============================================================================================================
// INTERNAL: Retrieves the socket record by the specified identifier
//============================================================================================================

Network::Socket* Network::_GetSocket (uint id)
{
	Socket* socket = 0;
	uint index = id & 0xFFFF;
	
	if (index < mSockets.GetSize())
	{
		Socket& s (mSockets[index]);
	
		// ID may have changed if the socket has been re-created since then
		if (s.mId == id) socket = &s;
	}
	return socket;
}

//============================================================================================================
// INTERNAL: Closes the specified socket with a different remote address (used for UDP sockets)
//============================================================================================================

bool Network::_CloseSocketByID (const Address& remote, uint id)
{
	mSockets.Lock();
	{
		Socket* socket = _GetSocket(id);

		if (socket != 0)
		{
			VoidPtr userData (socket->mUserData);
			::CloseSocket(*socket);
			mSockets.Unlock();
			
			// Inform the callback that the socket has been closed
			if (mOnClose) mOnClose(remote, id, userData);
			return true;
		}
	}
	mSockets.Unlock();
	return false;
}

//============================================================================================================
// INTERNAL: Retrieves an unused socket, creating one if necessary
//============================================================================================================

Socket& Network::_GetUnusedSocket()
{
	static uint counter = 0;

	// Run through all available sockets and find an unused one
	for (uint i = 0; i < mSockets.GetSize(); ++i)
	{
		Socket& s (mSockets[i]);

		if (s.mSocket == Socket::Invalid)
		{
			uint prefix = (++counter) & 0xFFFF;
			if (prefix == 0) { counter = 1; prefix = 1; }
			s.mId = (prefix << 16) | i;
			return s;
		}
	}

	// No unused socket found -- add a new one
	uint index = mSockets.GetSize();
	Socket& s (mSockets.Expand());

	uint prefix = (++counter) & 0xFFFF;
	if (prefix == 0) { counter = 1; prefix = 1; }
	s.mId = (prefix << 16) | index;
	return s;
}

//============================================================================================================
// INTERNAL: Establishes a connection with the topmost entry in the 'mConnect' array
//============================================================================================================

void Network::_Connect()
{
	ConnectData cd;

	// Remove one entry from the 'mConnect' array
	mConnect.Lock();
	{
		// This should never happen, but can't hurt just in case
		if (mConnect.GetSize() == 0)
		{
			mConnect.Unlock();
			return;
		}

		// Get the connection data and shrink the array, effectively removing it
		uint index = mConnect.GetSize() - 1;
		cd = mConnect[index];
		mConnect.Shrink();
	}
	mConnect.Unlock();

	// Resolve the address
	Network::Address ip = Resolve(cd.mAddress);

	// Start off with an invalid result
	uint result (-1);

	if (ip.IsValid())
	{
		// Get the 'sockaddr'
		sockaddr_in addr;
		AddressToSockaddr(ip, addr);

		// Connect to the requested address -- (0) is the return value on success
		result = ::connect(cd.mSocket, (sockaddr*)&addr, sizeof(addr));
	}

	// Try to connect to the destination (EINPROGRESS means the result will be known later)
	if ( result != 0 && result != EINPROGRESS )
	{
		// Trigger the OnError callback saying that the connection could not be established
		if (mOnError) mOnError(ip, cd.mId, cd.mUserData, "Destination unreachable");

		// On failure, lock the sockets list and close the socket
		mSockets.Lock();
		{
			Socket* socket = _GetSocket(cd.mId);
			if (socket != 0) ::CloseSocket(*socket);
		}
		mSockets.Unlock();
	}
	else
	{
		Socket* socket (0);

		// Find the newly connected socket
		mSockets.Lock();
		{
			socket = _GetSocket(cd.mId);
			ASSERT(socket != 0, "Socket not found");

			if (socket != 0)
			{
				socket->mAddress = ip;
				socket->mUserData = cd.mUserData;
				socket->mAction = Socket::Action::Ready;
			}
		}
		mSockets.Unlock();

		// Inform the listener
		if (mOnConnect) mOnConnect(ip, cd.mId, socket->mUserData, cd.mAddress);

		// In debug mode add an entry to the log file, logging the connection information
		DEBUG_LOG( String("Connected to %s", cd.mAddress.GetBuffer()) );
	}
}

//============================================================================================================
// INTERNAL: Sends specified data through the socket to the destination address
//============================================================================================================

bool Network::_Send (const void* data, uint length, const Socket& s, const Address& destination)
{
	uint sent (0);
	sockaddr_in addr;

	// UDP sockets need to have a valid address to send the data to
	if (s.mType == Socket::Type::UDP)
	{
		AddressToSockaddr(destination, addr);
	}

	while (sent < length)
	{
		// According to documentation, address portion is ignored for TCP sockets. It's only used for UDP sockets.
		int current = ::sendto(s.mSocket, ((const char*)data) + sent, length - sent, 0,
			(sockaddr*)&addr, sizeof(sockaddr_in));

		// Return value of '-1' means an error has occured
		if (current == -1)
		{
			int err = _GetError();

			if (err == EAGAIN || err == EWOULDBLOCK)
			{
				Thread::Sleep(0);
			}
			else
			{
				if (s.mType == Socket::Type::TCP)
				{
					// Remember the socket address and ID for use below
					uint	destId	(s.mId);
					VoidPtr	userData(s.mUserData);
					Address	destAddr(s.mAddress);

					// Close the socket
					::CloseSocket(s);

					// Connection has been aborted -- inform the callback
					if (mOnClose) mOnClose(destAddr, destId, userData);
				}
				else if (mOnError) mOnError(destination, s.mId, s.mUserData, "Invalid address");
				return false;
			}
		}
		else
		{
			mStatistics.mSent += (ulong)current;
			sent += current;
		}
	}
	return (sent == length);
}

//============================================================================================================
// INTERNAL: Accept an incoming connection on the specified socket
//============================================================================================================

void Network::_Accept (const Socket& s)
{
	Address acceptedAddress;
	uint id (0);
	sockaddr_in addr;
	socklen_t len ( sizeof(sockaddr_in) );
	memset(&addr, 0, len);

	// The connection will be accepted into the actual 'mSockets' list, so it must be locked first
	mSockets.Lock();
	{
		// Get an unused socket and accept the incoming connection
		Socket& acceptedSocket (_GetUnusedSocket());
		acceptedSocket.mType	  =  Socket::Type::TCP;
		acceptedSocket.mUserData  = s.mUserData;
		acceptedSocket.mRecvStamp = Time::GetMilliseconds();
		acceptedSocket.mSocket	  = (Socket::Identifier)::accept(s.mSocket, (sockaddr*)&addr, &len);

		// Remember the remote address
		SockaddrToAddress(acceptedSocket.mAddress, addr);
		
		if (acceptedSocket.mSocket == Socket::Invalid)
		{
			::CloseSocket(acceptedSocket);
		}
		else
		{
			acceptedSocket.mAction = Socket::Action::Ready;
			acceptedAddress = acceptedSocket.mAddress;
			id = acceptedSocket.mId;
		}
	}
	mSockets.Unlock();

	if (id != 0 && mOnConnect)
	{
		VoidPtr userData = s.mUserData;
		
		// Trigger the "OnConnect" callback
		mOnConnect(acceptedAddress, id, userData, acceptedAddress.ToString());

		// If the user data has been changed, we need to adjust the socket's value
		if (userData != s.mUserData)
		{
			mSockets.Lock();
			{
				Socket* socket = _GetSocket(id);
				if (socket != 0) socket->mUserData = userData;
			}
			mSockets.Unlock();
		}
	}

	// In debug mode add an entry to the log file, logging the connection information
#ifdef _DEBUG
	String msg ("Connected to ");
	msg << acceptedAddress.ToString();
	DEBUG_LOG( msg );
#endif
}

//============================================================================================================
// INTERNAL: Receive data on the specified socket
//============================================================================================================

void Network::_Receive (const Socket& s, byte* buffer, uint bufferSize, Thread::IDType threadId)
{
	sockaddr_in addr;
	socklen_t len ( sizeof(sockaddr_in) );
	memset(&addr, 0, len);

	// Receive the data
	int bytes = ::recvfrom(s.mSocket, (char*)buffer, bufferSize, 0, (sockaddr*)&addr, &len);

	// UDP sockets update the address above, TCP sockets do not
	Address remote;

	// Figure out the remote address
	if (s.mType == Socket::Type::TCP) remote = s.mAddress;
	else SockaddrToAddress(remote, addr);

	if (bytes > 0)
	{
		s.mRecvStamp = Time::GetMilliseconds();
		mStatistics.mReceived += (ulong)bytes;

		// Received data -- inform the callback
		if (mOnReceive) mOnReceive(remote, s.mId, s.mUserData, buffer, bytes, threadId);
	}
	else if (bytes == -1)
	{
		int err = _GetError();

		if (err == EINPROGRESS || err == EALREADY)
		{
			// Already receiving... possible race condition
		}
		else if (err == EWOULDBLOCK || err == EAGAIN)
		{
			// Nothing to receive for now -- just let the next Network::Update() loop take care of it
		}
		else if (err == ETIMEDOUT)
		{
			if (mOnError) mOnError(remote, s.mId, s.mUserData, "Destination is not responding");

			if (s.mType == Socket::Type::TCP)
			{
				// TCP connection timed out -- close it
				_CloseSocketByID(remote, s.mId);
			}
		}
		else
		{
			// Connection is aborted
			if (err == ECONNRESET || err == ECONNABORTED)
			{
				if (s.mType == Socket::Type::TCP)
				{
					_CloseSocketByID(remote, s.mId);
				}
				else if (mOnError) mOnError(remote, s.mId, s.mUserData, "Destination unreachable");
			}
			else // Some other error occured, forcing the socket to be closed
			{
				_CloseSocketByID(remote, s.mId);
			}
		}
	}
	else if (s.mType == Socket::Type::TCP)
	{
		// 0 bytes indicates the connection was closed
		_CloseSocketByID(remote, s.mId);
	}
}

//============================================================================================================
// Endless receiving loop
//============================================================================================================

void Network::_Update()
{
	// Increment the number of active threads running the Update loop
	Thread::Increment(mThreadCount);
	Thread::IDType threadId (Thread::GetID());

	// It's always useful to know when new threads are created and destroyed
	NORMAL_LOG( String("Worker thread %u has been created", threadId) );

	fd_set read;
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 0;

	// Temporary socket list that will be used by this thread
	SocketList temp;

	// Allocate a new receive buffer to be used by this thread
	byte* buffer = new byte[mBufferSize];

	while (!mTerminate)
	{
		bool active = false;
		uint offset = 0;
		uint end = mSockets.GetSize();

		// Run through all sockets in the list, processing up to FD_SETSIZE sockets at a time
		for (;;)
		{
			FD_ZERO(&read);
			uint largestSocket = 0;
			uint count = 0;

			// Copy a chunk of sockets into our temporary list
			mSockets.Lock();
			{
				// Clear the previous loop's "is receiving" flags
				for (uint i = 0; i < temp.GetSize(); ++i)
					mSockets[temp[i].mId & 0xFFFF].mIsReceiving = false;

				// Clear the temporary socket list
				temp.Clear();

				// Offset has reached the end of the socket list -- break out
				if (offset >= end)
				{
					mSockets.Unlock();
					break;
				}

				// Number of sockets grabbed by each ::select call is evenly split between active threads
				uint selectSize = mSockets.GetSize() / (uint)mThreadCount;

				// Select size should be no less than 16 and no higher than FD_SETSIZE
				if (selectSize < 16)		 selectSize = 16;
				if (selectSize > FD_SETSIZE) selectSize = FD_SETSIZE;

				for (; offset < end; ++offset)
				{
					Socket& s (mSockets[offset]);

					// Only worry about valid sockets
					if (s.IsReadyToReceive())
					{
						// Flag it for receiving
						s.mIsReceiving = true;

						// Add this socket to the temp list and set
#ifdef _WINDOWS
						// Windows FD_SET macro is inefficient and slow
						read.fd_array[count] = s.mSocket;
#else
						// Unix/MacOSX FD_SET macro on the other hand is perfectly fine
						FD_SET(s.mSocket, &read);
#endif
						++count;
						temp.Expand() = s;

						// Some implementations need to know the largest socket used for ::select()?
						if (s.mSocket > largestSocket)
							largestSocket = s.mSocket;

						// Socket set has been filled -- break out and process them
						if (count == selectSize)
							break;
					}
				}

				// No point in continuing if there are no valid sockets to work with
				if (count > 0)
				{
#ifdef _WINDOWS
					read.fd_count = count;
#endif
					// Select all sockets that have data waiting
					count = ::select(largestSocket+1, &read, NULL, NULL, &t);
				}
			}
			mSockets.Unlock();

			// If even one Select call found something to work with, count this pass as 'active'
			if (count > 0)
			{
				// Run through all the temporary sockets and process events
				for (uint i = 0; i < temp.GetSize(); ++i)
				{
					const Socket& s (temp[i]);
				
					// If this socket is in the set, do something
					if ( FD_ISSET(s.mSocket, &read) )
					{
						if (s.mAction == Socket::Action::Listening)
						{
							// Listener TCP sockets just accept incoming connections
							_Accept(s);
							active = true;
						}
						else if (s.mAction == Socket::Action::Ready)
						{
							// Sockets that are ready to receive data do just that
							_Receive(s, buffer, mBufferSize, threadId);

							// TCP packets are streamed, so they should not count as active for the sake of sleeping
							if (s.mType != Socket::Type::TCP) active = true;
						}
						else
						{
							NORMAL_LOG( String("ERROR! ::select() call returned socket %u (invalid state)", s.mSocket) );
						}
					}
				}
			}
		}

		// Sleep a bit, letting other threads do their work
		if (!mTerminate)
		{
			if (active)	{ if (mActiveSleep != -1) Thread::Sleep(mActiveSleep); }
			else		{ if (mIdleSleep   != -1) Thread::Sleep(mIdleSleep);	}
		}
	}

	// Forgetting to release memory would suck, wouldn't it?
	delete [] buffer;

	// It's always useful to know when new threads are created and destroyed
	NORMAL_LOG( String("Worker thread %u finished", Thread::GetID()) );

	// If this point is reached, then 'mTerminate' has been set to 'true'
	Thread::Decrement(mThreadCount);
}

//============================================================================================================
// Constructor and destructor initialize/release WinSock on windows side
//============================================================================================================

Network::Network() :	mThreadCount	(0),
						mTerminate		(false),
						mActiveSleep	(-1),
						mIdleSleep		(1),
						mBufferSize		(2048)
{
	NORMAL_LOG( String("Starting up the network") );

#ifdef _WINDOWS
	WSADATA wsaData;
	int err = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	ASSERT(err == 0, "Failed to initialize WinSock!");
#endif
}

//============================================================================================================

Network::~Network()
{
	Shutdown();

#ifdef _WINDOWS
	::WSACleanup();
#endif

	NORMAL_LOG( String("The network has been shut down") );
}

//============================================================================================================
// Starts listening for incoming connections or data on the specified port
//============================================================================================================

uint Network::Listen (ushort port, uint packetType, VoidPtr userData, int backlog)
{
	mSockets.Lock();

	// Ensure that we're not already using the specified port
	for (uint i = 0; i < mSockets.GetSize(); ++i)
	{
		Socket& s (mSockets[i]);

		if ( s.mAddress.mPort == port && s.mAction != Socket::Action::Disconnected )
		{
			// Port already in use -- just return a zero
			mSockets.Unlock();
			return 0;
		}
	}

	// Retrieve an unused socket entry
	Socket& s ( _GetUnusedSocket() );

	// Create an actual connection socket
	if (!::CreateSocket(s, packetType))
	{
		ASSERT(false, "Failed to create a socket");
		mSockets.Unlock();
		return 0;
	}

	// Set the IP address to any, and port to the specified value
	s.mAddress.mIp	 = 0;
	s.mAddress.mPort = port;
	s.mUserData		 = userData;

	// Convert our address into 'sockaddr_in' packetType that ::bind call understands
	sockaddr_in addr;
	AddressToSockaddr(s.mAddress, addr);

	// Bind the socket to the specified address/port
	if ( ::bind(s.mSocket, (sockaddr*)&addr, sizeof(addr)) != 0 )
	{
		// If ::bind() call fails, it means the port is already in use -- just return zero
		::CloseSocket(s);
		mSockets.Unlock();
		return 0;
	}

	if (packetType == Socket::Type::TCP)
	{
		// In case of TCP sockets, a call to ::listen() must be initiated to mark the connection as passive-listening
		if ( ::listen(s.mSocket, backlog) != 0 )
		{
			ASSERT(false, "::listen call failed");
			::CloseSocket(s);
			mSockets.Unlock();
			return 0;
		}

		s.mAction = Socket::Action::Listening;
		NORMAL_LOG( String("Listening for incoming TCP connections on port %u, socket %u", port, s.mSocket) );
	}
	else
	{
		// No further action is required for UDP sockets
		s.mAction	= Socket::Action::Ready;
		NORMAL_LOG( String("Listening for incoming UDP packets on port %u, socket %u", port, s.mSocket) );
	}

	// Return the socket identifier
	uint id = s.mId;
	mSockets.Unlock();
	return id;
}

//============================================================================================================
// Opens a TCP connection to the specified address
//============================================================================================================

void Network::Connect (const String& addr, VoidPtr userData)
{
	mSockets.Lock();
	{
		Socket& s ( _GetUnusedSocket() );

		// Create a new TCP socket
		if (!::CreateSocket(s, Socket::Type::TCP))
		{
			mSockets.Unlock();
			if (mOnError) mOnError(Address(), 0, userData, "Unable to create a socket!");
			return;
		}

		// Mark the socket as 'connecting'
		s.mAction = Socket::Action::Connecting;
		s.mUserData = userData;

		// Add a new entry to the connection list that the connection thread can later pick up
		mConnect.Lock();
		{
			ConnectData& data	= mConnect.Expand();
			data.mAddress		= addr;
			data.mSocket		= s.mSocket;
			data.mId			= s.mId;
			data.mUserData		= s.mUserData;
		}
		mConnect.Unlock();

		// Spawn a new connect thread
		Thread::Create(&Connect_Thread, this);
	}
	mSockets.Unlock();
}

//============================================================================================================
// Close the specified socket
//============================================================================================================

void Network::Close (uint id)
{
	mSockets.Lock();
	{
		Socket* socket = _GetSocket(id);

		if (socket != 0)
		{
			// Remember the address for the 'mOnClose' callback below
			Address remote (socket->mAddress);
			VoidPtr userData (socket->mUserData);
			::CloseSocket(*socket);
			mSockets.Unlock();
			
			// Inform the callback that the socket has been closed
			if (mOnClose) mOnClose(remote, id, userData);
			return;
		}
	}
	mSockets.Unlock();
}

//============================================================================================================
// Closes the specified socket
//============================================================================================================

//void Network::Close (const Socket& socket, bool threadSafe)
//{
//	if (socket.mId != Socket::Invalid)
//	{
//		if (threadSafe) mSockets.Lock();
//		uint id = socket.mId;
//		Address remote (socket.mAddress);
//		VoidPtr userData (socket.mUserData);
//		::CloseSocket(socket);
//		if (threadSafe) mSockets.Unlock();
//		if (mOnClose) mOnClose(remote, id, userData);
//	}
//}

//============================================================================================================
// Closes all active connections
//============================================================================================================

void Network::Disconnect()
{
	mSockets.Lock();
	{
		while (mSockets.IsValid())
		{
			Socket& socket = mSockets.Back();
			Address remote (socket.mAddress);
			VoidPtr userData (socket.mUserData);
			uint id (socket.mId);

			// Close the socket
			::CloseSocket(socket);
			mSockets.Shrink();
			mSockets.Unlock();

			// Inform the callback that the socket has been closed
			if (mOnClose) mOnClose(remote, id, userData);
			mSockets.Lock();
		}
		mSockets.Clear();
	}
	mSockets.Unlock();
}

//============================================================================================================
// Sends out data through the specified socket to the destination address
//============================================================================================================

bool Network::Send (const void* data, uint length, uint id, const Address& destination)
{
	bool retVal = false;
	if (length > 0 && data != 0)
	{
		mSockets.Lock();
		{
			if (id == 0)
			{
				for (uint i = 0; i < mSockets.GetSize(); ++i)
				{
					Socket& s (mSockets[i]);

					if ( s.IsReadyToSend() && s.mType == Socket::Type::TCP )
					{
						retVal |= _Send(data, length, s, destination);
					}
				}
			}
			else
			{
				Socket* socket = _GetSocket(id);

				if (socket != 0)
				{
					retVal = Send(data, length, *socket, destination);
				}
			}
		}
		mSockets.Unlock();
	}
	return retVal;
}

//============================================================================================================
// When sending data to a specific list of sockets, this function can do that with minimum overhead
//============================================================================================================

bool Network::Send (const void* data, uint length, const Array<uint>& sockets)
{
	bool retVal = false;
	if (length > 0 && data != 0)
	{
		mSockets.Lock();
		{
			Address destination;

			for (uint i = 0, imax = sockets.GetSize(); i < imax; ++i)
			{
				Socket* socket = _GetSocket(sockets[i]);

				if ( socket != 0 && socket->IsReadyToSend() && socket->mType == Socket::Type::TCP )
				{
					retVal |= _Send(data, length, *socket, destination);
				}
			}
		}
		mSockets.Unlock();
	}
	return retVal;
}

//============================================================================================================
// Adds a new worker thread that will run in the background and process messages
//============================================================================================================

void Network::SpawnWorkerThread()
{
	mTerminate = false;
	Thread::Create(Update_Thread, this);
}

//============================================================================================================
// Shuts down all threads and closes all active sockets
//============================================================================================================

void Network::Shutdown()
{
	TerminateAllThreads();

	mSockets.Lock();
	{
		for (uint i = 0; i < mSockets.GetSize(); ++i)
			::CloseSocket(mSockets[i]);
	}
	mSockets.Unlock();
}

//============================================================================================================
// Resolves the specified address (WARNING: It may take some time before this function returns!)
//------------------------------------------------------------------------------------------------------------
// NOTE: This function can be static as it doesn't access anything inside the class.
//		 I am keeping it non-static to ensure that the network gets initialized first, however.
//============================================================================================================

Network::Address Network::Resolve (const String& addr)
{
	Network::Address out;

	if (addr.IsValid())
	{
		uint a, b, c, d, port;

		// Check to see if an IP address was specified
		if ( sscanf(addr.GetBuffer(), "%u.%u.%u.%u:%u", &a, &b, &c, &d, &port) == 5 )
		{
			out.mIp = (a << 24 | b << 16 | c << 8 | d);
			out.mPort = port;
		}
		else if ( sscanf(addr.GetBuffer(), "%u.%u.%u.%u", &a, &b, &c, &d) == 4 )
		{
			out.mIp = (a << 24 | b << 16 | c << 8 | d);
			out.mPort = 80;
		}
		else // The address must need to be resolved
		{
			String address, port;

			// Try to split the address (www.something.com:8080)
			if (!addr.Split(address, ':', port))
			{
				// Can't split the address -- looks like the port has not been provided.
				address = addr;
			}

			// Since 'gethostbyname' returns a memory pointer, it's only logical to access it one at a time
			static Thread::Lockable criticalSection;

			criticalSection.Lock();
			{
				// Now comes the blocking call that retrieves the host information
				hostent* host = gethostbyname(address.GetBuffer());

				// For convenience
				byte* ip = (host == 0) ? 0 : (byte*)host->h_addr_list[0];

				if (ip != 0)
				{
					int result = *(uint*)ip;

					// Invalid IP results in ADDR_INVALID, which is (-1). Check to make sure it didn't happen.
					if (result != -1)
					{
						out.mIp = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | (ip[3]);

						// If port was resolved earlier, use it. Otherwise assume the default port.
						if (port.IsValid()) port >> out.mPort;
						else out.mPort = 80;
					}
				}
			}
			criticalSection.Unlock();
		}
	}
	return out;
}
