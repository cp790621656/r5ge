#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Same as an array, but for pointers -- automatically deletes them
//============================================================================================================

template <typename Type>
class PointerArray : public Unfinished::BaseArray<Type*>
{
public:

	typedef Type* TypePtr;
	typedef Unfinished::BaseArray<Type*>	BASE;

	PointerArray()						: BASE() {}
	PointerArray(const PointerArray& in): BASE() { *this = in; }
	PointerArray(uint reserve)			: BASE(reserve) {}
	virtual ~PointerArray()				{ Release(); }

	void Clear()
	{
		for (uint i = 0; i < BASE::mSize; ++i)
		{
			if (BASE::mArray[i] != 0)
			{
				delete BASE::mArray[i];
				BASE::mArray[i] = 0;
			}
		}
		BASE::mSize = 0;
	}

	void Release()
	{
		if (BASE::mArray != 0)
		{
			Clear();
			delete [] BASE::mArray;
			BASE::mArray = 0;
		}

		BASE::mAllocated = 0;
	}

	void ExpandTo(uint count)
	{
		BASE::Reserve(count);
		if (BASE::mSize < count)
		{
			memset(BASE::mArray + BASE::mSize, 0, (count - BASE::mSize) * sizeof(Type*));
			BASE::mSize = count;
		}
	}

	Type* Shrink (bool release = true)
	{
		if (BASE::mSize != 0)
		{
			Type* ptr = BASE::mArray[--BASE::mSize];

			if (release)
			{
				delete ptr;
				ptr = 0;
			}

			BASE::mArray[BASE::mSize] = 0;
			return ptr;
		}
		return 0;
	}

	bool operator >> (TypePtr& out)
	{
		if (BASE::mSize == 0)
		{
			out = 0;
			return false;
		}
		else
		{
			out = BASE::mArray[--BASE::mSize];
			BASE::mArray[BASE::mSize] = 0;
			return true;
		}
	}

	void operator = (const PointerArray& in)
	{
		Clear();

		if (in.IsValid())
		{
			if ( (BASE::mSize = in.GetSize()) > BASE::mAllocated )
			{
				if (BASE::mArray != 0) delete [] BASE::mArray;
				BASE::mArray = new Type*[BASE::mAllocated = BASE::mSize];
			}

			uint memory = BASE::mSize * sizeof(Type*);
			memcpy(BASE::mArray, in.BASE::mArray, memory);
			memset((Type*)in.BASE::mArray, 0, memory);
		}
	}

	// Deletes the specified entry from the array
	bool Delete (Type* ptr)
	{
		if (ptr != 0)
		{
			if (Remove(ptr))
			{
				delete ptr;
				return true;
			}
		}
		return false;
	}

	// Deletes an entry at the specified index
	bool DeleteAt (uint index)
	{
		if ( BASE::mSize != 0 && index < BASE::mSize )
		{
			Type* ptr = BASE::mArray[index];
			BASE::RemoveAt(index);

			if (ptr != 0)
			{
				delete ptr;
				return true;
			}
		}
		return false;
	}

	// Basic sorting algorithm for values referenced via pointers
	void Sort()
	{
		bool sorted = false;

		TypePtr* start	= BASE::mArray;
		TypePtr* end	= BASE::mArray + BASE::mSize;

		// Run through the arrays and sort them front-to-back
		for (TypePtr* last = end; last > start && !sorted; )
		{
			--last;
			sorted = true;

			for (TypePtr* curr = start; curr < last; ++curr )
			{
				if ( *(*(curr+1)) < *(*curr) )
				{
					Swap<TypePtr>( *curr, *(curr+1) );
					sorted = false;
				}
			}
		}
	}
};