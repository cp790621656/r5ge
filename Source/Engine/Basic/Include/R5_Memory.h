#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic memory buffer
//============================================================================================================

class Memory
{
protected:

	byte*				mBuffer;	// Allocated memory buffer
	uint				mAllocated;	// Actual amount of memory allocated
	uint				mSize;		// Memory that's actually "used" by the buffer
	Thread::Lockable	mLock;

public:

	Memory() : mBuffer(0), mAllocated(0), mSize(0) {}
	Memory(uint size) : mBuffer(0), mAllocated(0), mSize(0) { Resize(size); mSize = 0; }
	~Memory() { if (mBuffer != 0) delete [] mBuffer; }

public:

	// Copying memory buffers doesn't actually copy -- it moves the memory
	Memory (const Memory& in);
	void operator = (const Memory& in);

public:

	void		Lock()			const	{ mLock.Lock();			}
	void		Unlock()		const	{ mLock.Unlock();		}
	byte*		GetBuffer()				{ return mBuffer;		}
	const byte*	GetBuffer()		const	{ return mBuffer;		}
	bool		IsValid()		const	{ return mSize != 0;	}
	uint		GetSize()		const	{ return mSize;			}
	uint		GetAllocated()	const	{ return mAllocated;	}
	void		Clear()					{ mSize = 0;			}

	// Always useful to have
	void MemSet (int val) const { if (mBuffer != 0) memset(mBuffer, val, mAllocated); }

	// Release the memory buffer
	void Release();

	// Reserve the specified amount of memory
	byte* Reserve (uint size);

	// Resizes the buffer to the specified size, keeping the data in place if possible
	byte* Resize (uint size) { Reserve(size); mSize = size; return mBuffer; }

	// Expand the buffer by the specified amount of bytes and return a pointer to that location
	byte* Expand (uint bytes);

	// Remove the specified number of bytes from the front of the buffer
	void Remove (uint size);

	// Convenience function
	void Set (const void* buffer, uint size) { memcpy(Resize(size), buffer, size); }

	// Append types that can't be templated
	int32*	Append (int val)			{ int32*  ptr = (int32*) Expand(4); *ptr = (int32) val; return ptr; }
	uint32*	Append (uint val)			{ uint32* ptr = (uint32*)Expand(4); *ptr = (uint32)val; return ptr; }
	void	Append (const char* s);
	void	Append (const String& s);
	void*	Append (const void* buffer, uint size);

	// Appends an integer as either 1 or 5 bytes, depending on its own size
	void AppendSize (uint val);

	// Extract types that can't be templated
	static bool Extract (ConstBytePtr& buffer, uint& size, int& val);
	static bool Extract (ConstBytePtr& buffer, uint& size, uint& val);
	static bool Extract (ConstBytePtr& buffer, uint& size, String& val);
	static bool Extract (ConstBytePtr& buffer, uint& size, void* data, uint bytes);

	// Extracts a 1 to 5 byte integer encoded with AppendSize()
	static bool ExtractSize (ConstBytePtr& buffer, uint& size, uint& val);

	// Templated Append function for all other types not covered by the functions above
	template <typename Type> Type* Append (const Type& v)
	{
		Type* ptr = (Type*)Expand(sizeof(Type));
		*ptr = v;
		return ptr;
	}

	// Templated function to do the reverse of the Append operation above
	template <typename Type> static bool Extract (ConstBytePtr& buffer, uint& size, Type& val)
	{
		uint needed = sizeof(Type);
		if (size < needed) return false;
		val = *(Type*)buffer;
		buffer += needed;
		size -= needed;
		return true;
	}

public:

	// Load the specified file fully into memory
	bool Load (const char* filename);

	// Dump the current buffer into the file
	bool Save (const char* filename);
};