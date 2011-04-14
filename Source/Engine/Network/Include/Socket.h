#pragma once

//============================================================================================================
//              R5 Network, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Network::Socket -- stores the socket descriptor, type of the socket, its current action, and address
// Author: Michael Lyashenko
//============================================================================================================

struct Socket
{
	static const uint Invalid = -1;
	typedef uint Identifier;

	struct Type
	{
		enum
		{
			UDP = 0,
			TCP
		};
	};

	struct Action
	{
		enum
		{
			Disconnected = 0,
			Ready,
			Listening,
			Connecting,
		};
	};

	Socket::Identifier	mSocket;
	uint				mType;
	uint				mAction;
	Network::Address	mAddress;
	bool				mIsReceiving;
	uint				mId;
	mutable VoidPtr		mUserData;
	mutable ulong		mRecvStamp;

	Socket() :	mSocket		(Socket::Invalid),
				mType		(Type::UDP),
				mAction		(Action::Disconnected),
				mIsReceiving(false),
				mId			(0),
				mUserData	(0),
				mRecvStamp	(0) {}

	void Reset()
	{
		mSocket			= Socket::Invalid;
		mType			= Type::UDP;
		mAction			= Action::Disconnected;
		mIsReceiving	= false;
		mAddress.mIp	= 0;
		mAddress.mPort	= 0;
		mId				= 0;
		mUserData		= 0;
		mRecvStamp		= 0;
	}

	bool IsReadyToSend() const
	{
		return	mSocket != Socket::Invalid &&
				mAction == Socket::Action::Ready;
	}

	bool IsReadyToReceive() const
	{
		return (!mIsReceiving) && (mSocket != Socket::Invalid) && (mAction == Socket::Action::Ready || mAction == Socket::Action::Listening);
	}
};
