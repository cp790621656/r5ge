#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Copying memory buffers doesn't actually copy -- it moves the memory
//============================================================================================================

Memory::Memory (const Memory& in)
{
	Memory& mem		= const_cast<Memory&>(in);
	mBuffer			= mem.mBuffer;
	mAllocated		= mem.mAllocated;
	mSize			= mem.mSize;
	mem.mBuffer		= 0;
	mem.mAllocated	= 0;
	mem.mSize		= 0;
}

//============================================================================================================

void Memory::operator = (const Memory& in)
{
	Release();
	Memory& mem		= const_cast<Memory&>(in);
	mBuffer			= mem.mBuffer;
	mAllocated		= mem.mAllocated;
	mSize			= mem.mSize;
	mem.mBuffer		= 0;
	mem.mAllocated	= 0;
	mem.mSize		= 0;
}

//============================================================================================================
// Release the memory buffer
//============================================================================================================

void Memory::Release()
{
	if (mBuffer != 0)
	{
		delete [] mBuffer;
		mBuffer		= 0;
		mAllocated	= 0;
		mSize		= 0;
	}
}

//============================================================================================================
// Reserve the specified amount of memory
//============================================================================================================

byte* Memory::Reserve (uint size)
{
	if (mAllocated < size)
	{
		if (mBuffer != 0)
		{
			// Start with twice as much memory as currently allocated
			mAllocated = (mAllocated << 1);

			// If we requested more memory, adjust accordingly
			if (mAllocated < size) mAllocated = size;

			// Allocate the new buffer and copy the old data over
			byte* newBuffer = new byte[mAllocated];
			if (mSize > 0) memcpy(newBuffer, mBuffer, mSize);
			delete [] mBuffer;
			mBuffer = newBuffer;
		}
		else
		{
			// Minimum buffer size is 1 kb
			mAllocated = (size < 1024) ? 1024 : size;
			mBuffer = new byte[mAllocated];
		}
	}
	return mBuffer;
}

//============================================================================================================
// Expand the buffer by the specified maount of bytes and return a pointer to that location
//============================================================================================================

byte* Memory::Expand (uint bytes)
{
	uint offset = mSize;
	Resize(mSize + bytes);
	return mBuffer + offset;
}

//============================================================================================================
// Remove the specified number of bytes from the front of the buffer
//============================================================================================================

void Memory::Remove (uint size)
{
	if (size > 0)
	{
		if (size < mSize)
		{
			memmove(mBuffer, mBuffer + size, mSize -= size);
		}
		else
		{
			mSize = 0;
		}
	}
}

//============================================================================================================
// Strings have to be prepended with a 32-bit integer indicating their length
//============================================================================================================

void Memory::Append (const char* s)
{
	uint length = strlen(s);
	AppendSize(length);
	Append(s, length);
}

//============================================================================================================
// Strings have to be prepended with a 32-bit integer indicating their length
//============================================================================================================

void Memory::Append (const String& s)
{
	uint length = s.GetLength();
	AppendSize(length);
	Append(s.GetBuffer(), length);
}

//============================================================================================================
// Appends the specified chunk of memory to the end of this one
//============================================================================================================

void* Memory::Append (const void* buffer, uint size)
{
	if (size > 0)
	{
		void* ptr = Expand(size);
		memcpy(ptr, buffer, size);
		return ptr;
	}
	return 0;
}

//============================================================================================================
// Appends an integer as either 1 or 5 bytes, depending on its own size
//============================================================================================================

void Memory::AppendSize (uint val)
{
	if (val < 255)
	{
		*(byte*)Expand(1) = (byte)val;
	}
	else
	{
		byte* buffer = Expand(5);
		*(byte*)buffer = 255;
		*(uint32*)(buffer + 1) = (uint32)val;
	}
}

//============================================================================================================
// Extracts an integer from the specified buffer -- integers are packed into 4 bytes
//============================================================================================================

bool Memory::Extract (ConstBytePtr& buffer, uint& size, int& val)
{
	if (size < 4) return false;
	val = *(int32*)buffer;
	buffer += 4;
	size -= 4;
	return true;
}

//============================================================================================================
// Extracts an unsigned integer from the specified buffer -- uints are packed into 4 bytes
//============================================================================================================

bool Memory::Extract (ConstBytePtr& buffer, uint& size, uint& val)
{
	if (size < 4) return false;
	val = *(uint32*)buffer;
	buffer += 4;
	size -= 4;
	return true;
}

//============================================================================================================
// Extracts a string from the buffer. Strings are preceded by a 32-bit integer storing their length.
//============================================================================================================

bool Memory::Extract (ConstBytePtr& buffer, uint& size, String& val)
{
	uint length;

	if (ExtractSize(buffer, size, length))
	{
		return Extract(buffer, size, val.Resize(length), length);
	}
	return false;
}

//============================================================================================================
// Extracts the specified amount of bytes from the buffer
//============================================================================================================

bool Memory::Extract (ConstBytePtr& buffer, uint& size, void* data, uint bytes)
{
	if (size < bytes) return false;

	if (bytes > 0)
	{
		memcpy(data, buffer, bytes);
		buffer += bytes;
		size -= bytes;
	}
	return true;
}

//============================================================================================================
// Extracts a 1 to 5 byte integer encoded with AppendSize()
//============================================================================================================

bool Memory::ExtractSize (ConstBytePtr& buffer, uint& size, uint& val)
{
	if (size < 1) return false;

	val = buffer[0];
	++buffer;
	--size;

	if (val == 255)
	{
		if (size < 4) return false;

		val = *(uint32*)buffer;
		buffer += 4;
		size -= 4;
	}
	return true;
}

//============================================================================================================
// Load the specified file fully into memory
//============================================================================================================

bool Memory::Load (const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == 0) return false;

	fseek(fp, 0, SEEK_END);
	uint size = (uint)ftell(fp);
	byte* buffer = Resize(size);

	if (size > 0)
	{
		fseek(fp, 0, SEEK_SET);
		fread(buffer, 1, size, fp);
	}

	fclose(fp);
	return true;
}

//============================================================================================================
// Dump the current buffer into the file
//============================================================================================================

bool Memory::Save (const char* filename)
{
	FILE* fp = fopen(filename, "wb");
	if (fp == 0) return false;
	
	if (mBuffer != 0 && mSize > 0)
		fwrite(mBuffer, 1, mSize, fp);

	fclose(fp);
	return true;
}