#pragma once

//============================================================================================================
//              R5 Network, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Network class implements all the basic functionality dealing with connecting, sending, and receiving data
//============================================================================================================
// NOTE: Default settings are configured for client side or small server communication. For large scale
// servers it is highly recommended to have 1-2 worker threads per processor core, set idle sleep to 1 or 0
// and raise the expected socket count to a value of maximum expected simultaneous connections. Changing the
// buffer size might also improve performance for large-scale servers.
//============================================================================================================

class Network
{
public:

	// Network::Address contains the IP and Port of the remote host
	#include "Address.h"

	// Network::Socket helper class
	#include "Socket.h"

	// Various callbacks types used by the network
	// Arguments: Address, Socket ID, User Data, <varies>
	typedef FastDelegate<void (const String&)> OnLog;
	typedef FastDelegate<void (const Address&, uint, VoidPtr&, const String&)> OnConnect;
	typedef FastDelegate<void (const Address&, uint, VoidPtr&)> OnClose;
	typedef FastDelegate<void (const Address&, uint, VoidPtr&, const byte*, uint, Thread::IDType)>	OnReceive;
	typedef FastDelegate<void (const Address&, uint, VoidPtr&, const char*)> OnError;

	struct Statistics
	{
		ulong	mSent;		// Number of bytes sent
		ulong	mReceived;	// Number of bytes received

		Statistics() : mSent(0), mReceived(0) {}
	};

protected:

	// Connection data, stored locally and accessed by connect threads.
	// Designed this way in order to prevent memory allocation / deallocation in different threads.
	struct ConnectData
	{
		String				mAddress;	// Address we want to connect to
		uint				mSocket;	// Copied socket identifier used by ::connect call
		Socket::Identifier	mId;		// ID of the socket used to connect
		VoidPtr				mUserData;	// User specified data associated with the socket
	};

public:

	// Socket and connection lists
	typedef Array<Socket>		SocketList;
	typedef Array<ConnectData>	ConnectList;

	friend R5_THREAD_FUNCTION(Connect_Thread, ptr);
	friend R5_THREAD_FUNCTION(Update_Thread, ptr);

protected:

	SocketList		mSockets;		// All sockets
	ConnectList		mConnect;		// Connections that are currently being established
	Thread::ValType	mThreadCount;	// Number of active worker threads
	bool			mTerminate;		// Whether to signal threads to terminate
	uint			mActiveSleep;	// How long to Thread::Wait() for after at least one message has been processed (Default: no waiting)
	uint			mIdleSleep;		// How long to Thread::Wait() for after there were no messages to process (Default: 5 ms)
	uint			mBufferSize;	// Receive buffer size
	Statistics		mStatistics;	// Various statistics

	OnLog			mOnLog;
	OnError			mOnError;
	OnConnect		mOnConnect;
	OnClose			mOnClose;
	OnReceive		mOnReceive;

private: // Internal section

	// Retrieves the code of the last error
	static int _GetError();

	// Retrieves the socket record by the specified identifier
	Socket* _GetSocket (uint id);

	// Closes the specified socket
	void _CloseSocket (Socket& s);
	bool _CloseSocketByID (const Address& remote, uint id);

	// Retrieves an unused socket, creating one if necessary
	Socket& _GetUnusedSocket();

	// Establishes a connection with the topmost entry in the 'mConnect' array
	void _Connect ();

	// Sends specified data through the socket to the destination address
	bool _Send (const void* data, uint length, const Socket& s, const Address& destination);

	// Accept an incoming connection on the specified socket
	void _Accept (const Socket& s);

	// Receive data on the specified socket
	void _Receive (const Socket& s, byte* buffer, uint bufferSize, Thread::IDType threadId);

	// Endless receiving loop
	void _Update();

public:

	Network();
	~Network();

public: // Delegate section

	void SetOnLog	  (const OnLog&		fnc) { mOnLog		= fnc; }
	void SetOnConnect (const OnConnect&	fnc) { mOnConnect	= fnc; }
	void SetOnClose	  (const OnClose&	fnc) { mOnClose		= fnc; }
	void SetOnReceive (const OnReceive&	fnc) { mOnReceive	= fnc; }
	void SetOnError	  (const OnError&	fnc) { mOnError		= fnc; }

public: // Basic functionality section

	// Starts listening for incoming connections or data on the specified port (0 = failure)
	// User data is passed with every connection that's established via this connection in case of TCP,
	// or any data that is received via this connection in case of UDP.
	uint Listen (ushort		port,
				 uint		packetType	= Socket::Type::TCP,
				 VoidPtr	userData	= 0,
				 int		backlog		= 10);

	// Opens a TCP connection with the specified address (OnConnect or OnError is triggered with the result)
	// User data is passed back along with the result (OnConnect, OnError, OnReceive, etc)
	void Connect (const String& addr, VoidPtr userData = 0);

	// Close the specified socket (OnClose is triggered if successful)
	void Close (uint id);
	//void Close (const Socket& socket, bool threadSafe = true);

	// Closes all active connections
	void Disconnect();

	// Sends out data through the specified socket to the destination address
	bool Send (const void* data, uint length, uint id = 0, const Address& destination = Address());

	// A more direct version of the above -- note that it has no locking/unlocking inside,
	// making it suitable for use within a locked list retrieved by the GetSocketList() function.
	bool Send (const void* data, uint length, const Socket& socket, const Address& destination = Address())
	{
		return (length > 0 && data != 0 && socket.IsReadyToSend()) ? _Send(data, length, socket, destination) : false;
	}

	// When sending data to a specific list of sockets, this function can do that with minimum overhead
	bool Send (const void* data, uint length, const Array<uint>& sockets);

	// Adds a new worker thread that will run in the background and process messages
	void SpawnWorkerThread();

	// Shuts down all threads and closes all active sockets
	void Shutdown();

	// Statistics retrieval
	const Statistics& GetStatistics() const { return mStatistics; }

public: // Advanced functionality section

	// Resolves the specified address (WARNING: It may take some time before this function returns!)
	Address Resolve (const String& addr);

	// Changes the thread sleep delay, if so desired (specify '-1' to disable yielding to other threads)
	void SetSleepDelay (uint active, uint idle) { mActiveSleep = active; mIdleSleep = idle; }

	// Only allow buffer size changes when there are no threads active
	void SetBufferSize (uint size) { if (mThreadCount == 0 && size > 0) mBufferSize = size; }

	// Allow to manually reserve up to the expected number of sockets, just in case
	void SetExpectedSocketCount (ushort val) { mSockets.Reserve(val); }

	// Just in case, it is a good idea to allow outside users to manually terminate all active worker threads
	void TerminateAllThreads() { mTerminate = true; while (mThreadCount > 0) Thread::Sleep(0); }

	// Retrieves a socket from the specified identifier
	const Socket* GetSocket (uint id) const { return (const_cast<Network*>(this))->_GetSocket(id); }

	// Retrieves the entire list of managed sockets, useful for the more direct version of the Send() function above
	// NOTE: Be sure to Lock() the returned list before using it!
	const SocketList& GetSocketList() const { return mSockets; }
};
