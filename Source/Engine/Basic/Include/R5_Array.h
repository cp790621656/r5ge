#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Normal array template, replacement for std::vector -- to be used with anything but pointers
//============================================================================================================

template <typename Type>
class Array : public Unfinished::BaseArray<Type>
{
public:

	typedef Unfinished::BaseArray<Type> BASE;

	Array()				: BASE() {}
	Array(uint reserve)	: BASE(reserve) {}

	inline void Clear()
	{
		if (BASE::mArray != 0)
		{
			BASE::mSize = 0;
		}
	}

	inline void Release()
	{
		if (BASE::mArray != 0)
		{
			delete [] BASE::mArray;
			BASE::mArray = 0;
		}
		BASE::mAllocated = 0;
		BASE::mSize = 0;
	}

	inline Type* ExpandTo(uint count)
	{
		BASE::Reserve(count);
		if (BASE::mSize < count) BASE::mSize = count;
		return BASE::mArray;
	}

	inline void Shrink()
	{
		if (BASE::mSize) --BASE::mSize;
	}

	inline bool operator >> (Type& out)
	{
		if (BASE::mSize == 0) return false;
		out = BASE::mArray[--BASE::mSize];
		return true;
	}

	inline void operator =(const Array<Type>& in)
	{
		Clear();
		
		if (in.IsValid())
		{
			uint size = in.GetSize();
			BASE::Reserve(size);
			for (uint i = 0; i < size; ++i) BASE::Expand() = in[i];
		}
	}

	inline void CopyMemory (const Array<Type>& in)
	{
		Clear();

		if (in.IsValid())
		{
			memcpy( ExpandTo(in.GetSize()),
					in.GetBuffer(),
					in.GetSizeInMemory() );
		}
	}

	// Basic sorting algorithm
	void Sort()
	{
		bool sorted = false;

		Type* start = BASE::mArray;
		Type* end	= BASE::mArray + BASE::mSize;

		// Run through the array's entries and sort them front-to-back
		for (Type* last = end; last > start && !sorted; )
		{
			--last;
			sorted = true;

			for (Type* curr = start; curr < last; ++curr )
			{
				if ( *(curr+1) < *curr )
				{
					Swap<Type>( *curr, *(curr+1) );
					sorted = false;
				}
			}
		}
	}

	// Adds a new entry in the correct spot in a previously sorted array.
	// Return value is the index of the newly added entry in the array.
	uint AddSorted (const Type& val)
	{
		// If there are no entries in the array, just add this entry to the end
		if (BASE::mSize == 0)
		{
			BASE::Expand() = val;
			return 0;
		}

		// Run through all entries and figure out where to insert this one
		uint index;
		for (index = 0; index < BASE::mSize; ++index) if (val < BASE::mArray[index]) break;

		if (BASE::mSize != index)
		{
			// Add a new entry
			BASE::Expand();

			// Run through all remaining entries, shifting them over by one
			for (uint i = BASE::mSize; i > index; ) { --i; BASE::mArray[i + 1] = BASE::mArray[i]; }

			// Assign the value
			BASE::mArray[index] = val;
		}
		else
		{
			// Simply append to the end
			BASE::Expand() = val;
		}
		return index;
	}

	inline bool SerializeTo (Memory& mem) const
	{
		uint bytes = BASE::mSize * sizeof(Type);
		mem.Append(bytes);
		mem.Append(BASE::mArray, bytes);
		return true;
	}

	inline bool SerializeFrom (ConstBytePtr& buffer, uint& size)
	{
		Clear();
		uint bytes;

		if (Memory::Extract(buffer, size, bytes))
		{
			return Memory::Extract(buffer, size, ExpandTo(bytes / sizeof(Type)), bytes);
		}
		return false;
	}
};