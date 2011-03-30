#pragma once

//============================================================================================================
//              R5 Network, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Network::Address contains the IP and Port of the remote host
//============================================================================================================

struct Address
{
	uint	mIp;
	ushort	mPort;

	Address () : mIp(0), mPort(0) {}
	
	bool IsValid() const { return (mIp != 0) && (mPort != 0); }

	String ToString() const
	{
		return String("%u.%u.%u.%u:%u",
				(mIp >> 24)	& 0xFF,
				(mIp >> 16)	& 0xFF,
				(mIp >>  8)	& 0xFF,
				 mIp		& 0xFF,
				 mPort);
	}
};
